/*
Copyright (C) 1996-1997 Id Software, Inc.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/
// cl_input.c -- Client input and movement prediction
// BUGGY VERSION - Contains prediction reconciliation bugs

#include "quakedef.h"

/*
 * Client-side prediction in QuakeWorld:
 *
 * The client maintains a circular buffer of predicted movement frames.
 * Each frame contains:
 *   - The input command (movement, buttons, etc.)
 *   - The resulting position/velocity after simulation
 *   - A sequence number for synchronization with server
 *
 * When a server update arrives:
 *   1. Find the corresponding predicted frame by sequence number
 *   2. Compare server position to predicted position
 *   3. If they differ significantly, correct the prediction
 *   4. Re-simulate from the corrected frame forward
 */

#define PREDICTION_BUFFER_SIZE 64
#define PREDICTION_BUFFER_MASK (PREDICTION_BUFFER_SIZE - 1)

typedef struct {
    int         sequence;       // Command sequence number
    usercmd_t   cmd;           // Input command for this frame
    vec3_t      origin;        // Position after this command
    vec3_t      velocity;      // Velocity after this command
    qboolean    valid;         // Is this frame valid?
} predicted_frame_t;

predicted_frame_t predicted_frames[PREDICTION_BUFFER_SIZE];

// Current prediction state
int     cl_predict_sequence;    // Next sequence to use
vec3_t  cl_predicted_origin;    // Current predicted position
vec3_t  cl_predicted_velocity;  // Current predicted velocity

// Server acknowledgment
int     cl_server_ack_sequence; // Last sequence server acknowledged


/*
===============
CL_InitPrediction

Initialize the prediction system
===============
*/
void CL_InitPrediction(void)
{
    int i;

    cl_predict_sequence = 1;
    cl_server_ack_sequence = 0;

    VectorClear(cl_predicted_origin);
    VectorClear(cl_predicted_velocity);

    for (i = 0; i < PREDICTION_BUFFER_SIZE; i++) {
        predicted_frames[i].valid = false;
        predicted_frames[i].sequence = 0;
    }
}


/*
===============
CL_StorePredictonFrame

Store a predicted frame in the circular buffer
===============
*/
void CL_StorePredictionFrame(int sequence, usercmd_t *cmd, vec3_t origin, vec3_t velocity)
{
    int slot = sequence & PREDICTION_BUFFER_MASK;
    predicted_frame_t *frame = &predicted_frames[slot];

    frame->sequence = sequence;
    frame->cmd = *cmd;
    VectorCopy(origin, frame->origin);
    VectorCopy(velocity, frame->velocity);
    frame->valid = true;
}


/*
===============
CL_FindPredictionFrame

Find a predicted frame by sequence number.
Returns NULL if not found.
===============
*/
predicted_frame_t *CL_FindPredictionFrame(int sequence)
{
    int slot = sequence & PREDICTION_BUFFER_MASK;
    predicted_frame_t *frame = &predicted_frames[slot];

    if (frame->valid && frame->sequence == sequence)
        return frame;

    return NULL;
}


/*
===============
CL_PredictMove

Simulate one tick of player movement using the given command.
This is a simplified version - real Quake does full physics.
===============
*/
void CL_PredictMove(usercmd_t *cmd, vec3_t origin, vec3_t velocity,
                    vec3_t out_origin, vec3_t out_velocity)
{
    float frametime = cmd->msec / 1000.0f;
    vec3_t wishvel;
    float friction = 4.0f;
    float accel = 10.0f;
    float maxspeed = 320.0f;

    // Apply friction
    float speed = VectorLength(velocity);
    if (speed > 0) {
        float drop = speed * friction * frametime;
        float newspeed = speed - drop;
        if (newspeed < 0)
            newspeed = 0;
        VectorScale(velocity, newspeed / speed, velocity);
    }

    // Calculate desired velocity from input
    wishvel[0] = cmd->forwardmove;
    wishvel[1] = cmd->sidemove;
    wishvel[2] = 0;

    // Clamp to max speed
    float wishspeed = VectorLength(wishvel);
    if (wishspeed > maxspeed) {
        VectorScale(wishvel, maxspeed / wishspeed, wishvel);
        wishspeed = maxspeed;
    }

    // Accelerate
    float currentspeed = DotProduct(velocity, wishvel) / (wishspeed + 0.001f);
    float addspeed = wishspeed - currentspeed;
    if (addspeed > 0) {
        float accelspeed = accel * frametime * wishspeed;
        if (accelspeed > addspeed)
            accelspeed = addspeed;
        VectorMA(velocity, accelspeed / (wishspeed + 0.001f), wishvel, velocity);
    }

    // Update position
    VectorMA(origin, frametime, velocity, out_origin);
    VectorCopy(velocity, out_velocity);
}


/*
===============
CL_ReconcilePrediction

Called when server update arrives. Compares server state to predicted
state and corrects if necessary.

BUG 1: Off-by-one in sequence number comparison
BUG 2: Incorrect position tolerance calculation
===============
*/
void CL_ReconcilePrediction(int server_sequence, vec3_t server_origin, vec3_t server_velocity)
{
    predicted_frame_t *frame;
    float tolerance;
    vec3_t delta;
    int i;

    // BUG 1: Off-by-one error - should be server_sequence, not server_sequence - 1
    // This causes us to compare against the wrong frame, leading to
    // unnecessary corrections and rubber-banding
    frame = CL_FindPredictionFrame(server_sequence - 1);

    if (!frame) {
        // Frame not found, just snap to server position
        VectorCopy(server_origin, cl_predicted_origin);
        VectorCopy(server_velocity, cl_predicted_velocity);
        return;
    }

    // Calculate position difference
    VectorSubtract(server_origin, frame->origin, delta);

    // BUG 2: Tolerance based on individual velocity components instead of magnitude
    // This causes high diagonal movement to trigger false corrections
    // Should be: tolerance = 0.1f + VectorLength(server_velocity) * 0.01f;
    tolerance = 0.1f + (fabs(server_velocity[0]) + fabs(server_velocity[1]) +
                        fabs(server_velocity[2])) * 0.01f;

    // Check if correction needed
    if (VectorLength(delta) > tolerance) {
        // Need to correct - re-simulate from server state
        vec3_t current_origin, current_velocity;
        int seq;

        VectorCopy(server_origin, current_origin);
        VectorCopy(server_velocity, current_velocity);

        // Re-simulate all frames from server_sequence to current
        // BUG 3: Loop starts at wrong sequence due to bug 1
        for (seq = server_sequence; seq < cl_predict_sequence; seq++) {
            predicted_frame_t *replay_frame = CL_FindPredictionFrame(seq);

            if (replay_frame && replay_frame->valid) {
                vec3_t new_origin, new_velocity;

                CL_PredictMove(&replay_frame->cmd, current_origin, current_velocity,
                              new_origin, new_velocity);

                // Update the stored prediction with corrected values
                VectorCopy(new_origin, replay_frame->origin);
                VectorCopy(new_velocity, replay_frame->velocity);

                VectorCopy(new_origin, current_origin);
                VectorCopy(new_velocity, current_velocity);
            }
        }

        // Update final predicted state
        VectorCopy(current_origin, cl_predicted_origin);
        VectorCopy(current_velocity, cl_predicted_velocity);
    }

    // Update acknowledgment
    cl_server_ack_sequence = server_sequence;
}


/*
===============
CL_ProcessInput

Process a new input command and update prediction
===============
*/
void CL_ProcessInput(usercmd_t *cmd)
{
    vec3_t new_origin, new_velocity;

    // Simulate movement
    CL_PredictMove(cmd, cl_predicted_origin, cl_predicted_velocity,
                   new_origin, new_velocity);

    // Store in prediction buffer
    CL_StorePredictionFrame(cl_predict_sequence, cmd, new_origin, new_velocity);

    // Update state
    VectorCopy(new_origin, cl_predicted_origin);
    VectorCopy(new_velocity, cl_predicted_velocity);

    cl_predict_sequence++;
}


/*
===============
CL_GetPredictedPosition

Get the current predicted position for rendering
===============
*/
void CL_GetPredictedPosition(vec3_t out)
{
    VectorCopy(cl_predicted_origin, out);
}
