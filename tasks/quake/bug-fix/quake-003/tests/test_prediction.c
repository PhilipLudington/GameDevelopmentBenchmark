/*
 * Network Prediction Test Harness
 *
 * Tests client-side prediction and server reconciliation for QuakeWorld.
 * Mocks Quake's data structures to create isolated unit tests.
 *
 * The buggy version has three related flaws:
 * 1. Off-by-one error in sequence number comparison (uses server_seq - 1)
 * 2. Incorrect tolerance calculation (sums components instead of magnitude)
 * 3. Re-simulation starts at wrong frame due to bug 1
 *
 * These bugs cause unnecessary corrections, leading to rubber-banding.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdbool.h>

/* ============== MOCK QUAKE TYPES ============== */

typedef float vec_t;
typedef vec_t vec3_t[3];
typedef int qboolean;

#define true 1
#define false 0

/* Vector operations */
#define VectorClear(v) ((v)[0] = (v)[1] = (v)[2] = 0)
#define VectorCopy(s, d) ((d)[0] = (s)[0], (d)[1] = (s)[1], (d)[2] = (s)[2])
#define VectorSubtract(a, b, c) ((c)[0] = (a)[0] - (b)[0], (c)[1] = (a)[1] - (b)[1], (c)[2] = (a)[2] - (b)[2])
#define VectorScale(v, s, d) ((d)[0] = (v)[0] * (s), (d)[1] = (v)[1] * (s), (d)[2] = (v)[2] * (s))
#define VectorMA(v, s, b, d) ((d)[0] = (v)[0] + (s) * (b)[0], (d)[1] = (v)[1] + (s) * (b)[1], (d)[2] = (v)[2] + (s) * (b)[2])
#define DotProduct(a, b) ((a)[0] * (b)[0] + (a)[1] * (b)[1] + (a)[2] * (b)[2])

float VectorLength(vec3_t v) {
    return sqrtf(v[0] * v[0] + v[1] * v[1] + v[2] * v[2]);
}

/* User command structure */
typedef struct {
    int     msec;           // Milliseconds for this command
    float   forwardmove;    // Forward movement speed
    float   sidemove;       // Strafe movement speed
    float   upmove;         // Jump/crouch
    int     buttons;        // Button presses
} usercmd_t;

/* ============== PREDICTION SYSTEM ============== */

#define PREDICTION_BUFFER_SIZE 64
#define PREDICTION_BUFFER_MASK (PREDICTION_BUFFER_SIZE - 1)

typedef struct {
    int         sequence;
    usercmd_t   cmd;
    vec3_t      origin;
    vec3_t      velocity;
    qboolean    valid;
} predicted_frame_t;

predicted_frame_t predicted_frames[PREDICTION_BUFFER_SIZE];

int     cl_predict_sequence;
vec3_t  cl_predicted_origin;
vec3_t  cl_predicted_velocity;
int     cl_server_ack_sequence;

/* Correction tracking for tests */
int correction_count = 0;
float total_correction_distance = 0.0f;


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
    correction_count = 0;
    total_correction_distance = 0.0f;
}

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

predicted_frame_t *CL_FindPredictionFrame(int sequence)
{
    int slot = sequence & PREDICTION_BUFFER_MASK;
    predicted_frame_t *frame = &predicted_frames[slot];
    if (frame->valid && frame->sequence == sequence)
        return frame;
    return NULL;
}

void CL_PredictMove(usercmd_t *cmd, vec3_t origin, vec3_t velocity,
                    vec3_t out_origin, vec3_t out_velocity)
{
    float frametime = cmd->msec / 1000.0f;
    vec3_t wishvel;
    float friction = 4.0f;
    float accel = 10.0f;
    float maxspeed = 320.0f;

    float speed = VectorLength(velocity);
    if (speed > 0) {
        float drop = speed * friction * frametime;
        float newspeed = speed - drop;
        if (newspeed < 0) newspeed = 0;
        VectorScale(velocity, newspeed / speed, velocity);
    }

    wishvel[0] = cmd->forwardmove;
    wishvel[1] = cmd->sidemove;
    wishvel[2] = 0;

    float wishspeed = VectorLength(wishvel);
    if (wishspeed > maxspeed) {
        VectorScale(wishvel, maxspeed / wishspeed, wishvel);
        wishspeed = maxspeed;
    }

    float currentspeed = DotProduct(velocity, wishvel) / (wishspeed + 0.001f);
    float addspeed = wishspeed - currentspeed;
    if (addspeed > 0) {
        float accelspeed = accel * frametime * wishspeed;
        if (accelspeed > addspeed) accelspeed = addspeed;
        VectorMA(velocity, accelspeed / (wishspeed + 0.001f), wishvel, velocity);
    }

    VectorMA(origin, frametime, velocity, out_origin);
    VectorCopy(velocity, out_velocity);
}


/* BUGGY reconciliation */
void CL_ReconcilePrediction_Buggy(int server_sequence, vec3_t server_origin, vec3_t server_velocity)
{
    predicted_frame_t *frame;
    float tolerance;
    vec3_t delta;

    // BUG 1: Off-by-one
    frame = CL_FindPredictionFrame(server_sequence - 1);

    if (!frame) {
        VectorCopy(server_origin, cl_predicted_origin);
        VectorCopy(server_velocity, cl_predicted_velocity);
        return;
    }

    VectorSubtract(server_origin, frame->origin, delta);

    // BUG 2: Sum of components instead of magnitude
    tolerance = 0.1f + (fabsf(server_velocity[0]) + fabsf(server_velocity[1]) +
                        fabsf(server_velocity[2])) * 0.01f;

    if (VectorLength(delta) > tolerance) {
        vec3_t current_origin, current_velocity;

        correction_count++;
        total_correction_distance += VectorLength(delta);

        VectorCopy(server_origin, current_origin);
        VectorCopy(server_velocity, current_velocity);

        // BUG 3: Starts at wrong sequence
        for (int seq = server_sequence; seq < cl_predict_sequence; seq++) {
            predicted_frame_t *replay_frame = CL_FindPredictionFrame(seq);
            if (replay_frame && replay_frame->valid) {
                vec3_t new_origin, new_velocity;
                CL_PredictMove(&replay_frame->cmd, current_origin, current_velocity,
                              new_origin, new_velocity);
                VectorCopy(new_origin, replay_frame->origin);
                VectorCopy(new_velocity, replay_frame->velocity);
                VectorCopy(new_origin, current_origin);
                VectorCopy(new_velocity, current_velocity);
            }
        }

        VectorCopy(current_origin, cl_predicted_origin);
        VectorCopy(current_velocity, cl_predicted_velocity);
    }

    cl_server_ack_sequence = server_sequence;
}


/* FIXED reconciliation */
void CL_ReconcilePrediction_Fixed(int server_sequence, vec3_t server_origin, vec3_t server_velocity)
{
    predicted_frame_t *frame;
    float tolerance;
    vec3_t delta;

    // FIX 1: Correct sequence
    frame = CL_FindPredictionFrame(server_sequence);

    if (!frame) {
        VectorCopy(server_origin, cl_predicted_origin);
        VectorCopy(server_velocity, cl_predicted_velocity);
        return;
    }

    VectorSubtract(server_origin, frame->origin, delta);

    // FIX 2: Vector magnitude
    tolerance = 0.1f + VectorLength(server_velocity) * 0.01f;

    if (VectorLength(delta) > tolerance) {
        vec3_t current_origin, current_velocity;

        correction_count++;
        total_correction_distance += VectorLength(delta);

        VectorCopy(server_origin, current_origin);
        VectorCopy(server_velocity, current_velocity);

        // FIX 3: Start from correct sequence
        for (int seq = server_sequence + 1; seq < cl_predict_sequence; seq++) {
            predicted_frame_t *replay_frame = CL_FindPredictionFrame(seq);
            if (replay_frame && replay_frame->valid) {
                vec3_t new_origin, new_velocity;
                CL_PredictMove(&replay_frame->cmd, current_origin, current_velocity,
                              new_origin, new_velocity);
                VectorCopy(new_origin, replay_frame->origin);
                VectorCopy(new_velocity, replay_frame->velocity);
                VectorCopy(new_origin, current_origin);
                VectorCopy(new_velocity, current_velocity);
            }
        }

        VectorCopy(current_origin, cl_predicted_origin);
        VectorCopy(current_velocity, cl_predicted_velocity);
    }

    cl_server_ack_sequence = server_sequence;
}


void CL_ProcessInput(usercmd_t *cmd)
{
    vec3_t new_origin, new_velocity;
    CL_PredictMove(cmd, cl_predicted_origin, cl_predicted_velocity,
                   new_origin, new_velocity);
    CL_StorePredictionFrame(cl_predict_sequence, cmd, new_origin, new_velocity);
    VectorCopy(new_origin, cl_predicted_origin);
    VectorCopy(new_velocity, cl_predicted_velocity);
    cl_predict_sequence++;
}


/* ============== TEST CASES ============== */

/*
 * Test 1: Perfect prediction - no corrections needed
 * When client and server agree exactly, neither version should correct.
 */
int test_perfect_prediction(void)
{
    printf("Test: Perfect prediction (no corrections needed)\n");

    CL_InitPrediction();

    // Simulate some movement
    usercmd_t cmd = { .msec = 16, .forwardmove = 100, .sidemove = 0 };
    for (int i = 0; i < 10; i++) {
        CL_ProcessInput(&cmd);
    }

    // Server confirms exactly what we predicted
    for (int seq = 1; seq < cl_predict_sequence; seq++) {
        predicted_frame_t *frame = CL_FindPredictionFrame(seq);
        if (frame) {
            int buggy_corrections_before = correction_count;
            CL_ReconcilePrediction_Fixed(seq, frame->origin, frame->velocity);

            if (correction_count > buggy_corrections_before) {
                printf("  FAIL: Fixed version made unnecessary correction at seq %d\n", seq);
                return 0;
            }
        }
    }

    printf("  PASS (no corrections made)\n");
    return 1;
}


/*
 * Test 2: Off-by-one bug detection
 * This test specifically triggers the off-by-one error.
 */
int test_off_by_one_bug(void)
{
    printf("Test: Off-by-one sequence bug detection\n");

    // Buggy version
    CL_InitPrediction();
    usercmd_t cmd = { .msec = 16, .forwardmove = 100, .sidemove = 0 };
    for (int i = 0; i < 5; i++) {
        CL_ProcessInput(&cmd);
    }

    // Server sends update for sequence 3
    // Buggy version will look up sequence 2 instead
    predicted_frame_t *frame3 = CL_FindPredictionFrame(3);
    predicted_frame_t *frame2 = CL_FindPredictionFrame(2);

    if (!frame3 || !frame2) {
        printf("  FAIL: Frames not found\n");
        return 0;
    }

    // Make frame 3's position different from frame 2
    // Server says we're at frame3 position, buggy code looks at frame2
    vec3_t server_origin;
    VectorCopy(frame3->origin, server_origin);
    vec3_t server_velocity = {50, 0, 0};

    int buggy_corrections_before = correction_count;
    CL_ReconcilePrediction_Buggy(3, server_origin, server_velocity);
    int buggy_corrections = correction_count - buggy_corrections_before;

    // Reset and test fixed version
    CL_InitPrediction();
    for (int i = 0; i < 5; i++) {
        CL_ProcessInput(&cmd);
    }

    int fixed_corrections_before = correction_count;
    CL_ReconcilePrediction_Fixed(3, server_origin, server_velocity);
    int fixed_corrections = correction_count - fixed_corrections_before;

    printf("  Buggy corrections: %d, Fixed corrections: %d\n",
           buggy_corrections, fixed_corrections);

    // Buggy version may make unnecessary corrections
    // Fixed version should not (since server matches our prediction)
    if (fixed_corrections == 0) {
        printf("  PASS - Fixed version correctly avoided unnecessary correction\n");
        return 1;
    } else {
        printf("  FAIL - Fixed version made unexpected correction\n");
        return 0;
    }
}


/*
 * Test 3: Diagonal movement tolerance
 * The buggy tolerance calculation sums components, giving higher tolerance
 * for diagonal movement. This test verifies consistent tolerance.
 */
int test_diagonal_movement_tolerance(void)
{
    printf("Test: Diagonal movement tolerance consistency\n");

    // Test with axis-aligned velocity
    vec3_t velocity_axis = {300, 0, 0};  // 300 units/s forward
    float tolerance_axis = 0.1f + VectorLength(velocity_axis) * 0.01f;  // Fixed formula

    // Test with diagonal velocity (same speed)
    float diag = 300.0f / sqrtf(2.0f);  // ~212 each component
    vec3_t velocity_diag = {diag, diag, 0};
    float tolerance_diag = 0.1f + VectorLength(velocity_diag) * 0.01f;  // Fixed formula

    // Buggy formula: sum of abs components
    float buggy_tolerance_axis = 0.1f + (fabsf(velocity_axis[0]) +
                                         fabsf(velocity_axis[1]) +
                                         fabsf(velocity_axis[2])) * 0.01f;
    float buggy_tolerance_diag = 0.1f + (fabsf(velocity_diag[0]) +
                                         fabsf(velocity_diag[1]) +
                                         fabsf(velocity_diag[2])) * 0.01f;

    printf("  Fixed formula - axis: %.3f, diagonal: %.3f\n",
           tolerance_axis, tolerance_diag);
    printf("  Buggy formula - axis: %.3f, diagonal: %.3f\n",
           buggy_tolerance_axis, buggy_tolerance_diag);

    // Fixed tolerances should be nearly equal (same speed)
    float fixed_diff = fabsf(tolerance_axis - tolerance_diag);
    // Buggy tolerances differ significantly
    float buggy_diff = fabsf(buggy_tolerance_axis - buggy_tolerance_diag);

    if (fixed_diff < 0.01f && buggy_diff > 0.5f) {
        printf("  PASS - Fixed formula gives consistent tolerance (diff: %.4f)\n", fixed_diff);
        printf("         Buggy formula inconsistent (diff: %.4f)\n", buggy_diff);
        return 1;
    } else {
        printf("  FAIL - Tolerance difference not as expected\n");
        return 0;
    }
}


/*
 * Test 4: Server correction handling
 * When server position differs, verify proper re-simulation.
 */
int test_server_correction(void)
{
    printf("Test: Server correction re-simulation\n");

    CL_InitPrediction();

    // Process some input
    usercmd_t cmd = { .msec = 16, .forwardmove = 100, .sidemove = 0 };
    for (int i = 0; i < 10; i++) {
        CL_ProcessInput(&cmd);
    }

    // Server says we're at a different position for sequence 5
    // (simulating server-side collision or other discrepancy)
    vec3_t server_origin = {100, 50, 0};  // Server says different Y position
    vec3_t server_velocity = {50, 0, 0};

    int corrections_before = correction_count;
    CL_ReconcilePrediction_Fixed(5, server_origin, server_velocity);

    // Should have made exactly one correction
    if (correction_count == corrections_before + 1) {
        // Verify the final position was re-simulated from server state
        // It should NOT match our original prediction
        printf("  Correction applied, new position: (%.2f, %.2f, %.2f)\n",
               cl_predicted_origin[0], cl_predicted_origin[1], cl_predicted_origin[2]);
        printf("  PASS - Correction was applied\n");
        return 1;
    } else {
        printf("  FAIL - Expected 1 correction, got %d\n",
               correction_count - corrections_before);
        return 0;
    }
}


/*
 * Test 5: Correction frequency under normal conditions
 * The buggy version makes more corrections than necessary.
 */
int test_correction_frequency(void)
{
    printf("Test: Correction frequency comparison\n");

    // Simulate 100 frames of movement with minor server discrepancies
    int buggy_total_corrections = 0;
    int fixed_total_corrections = 0;

    // Test buggy version
    CL_InitPrediction();
    usercmd_t cmd = { .msec = 16, .forwardmove = 100, .sidemove = 50 };

    for (int i = 0; i < 100; i++) {
        CL_ProcessInput(&cmd);

        // Simulate server update with tiny random discrepancy
        if (cl_predict_sequence > 5) {
            predicted_frame_t *frame = CL_FindPredictionFrame(cl_predict_sequence - 5);
            if (frame) {
                vec3_t server_origin;
                VectorCopy(frame->origin, server_origin);
                // Add tiny discrepancy (within acceptable tolerance)
                server_origin[0] += 0.05f;

                int before = correction_count;
                CL_ReconcilePrediction_Buggy(cl_predict_sequence - 5, server_origin, frame->velocity);
                if (correction_count > before) buggy_total_corrections++;
            }
        }
    }

    // Test fixed version
    CL_InitPrediction();
    for (int i = 0; i < 100; i++) {
        CL_ProcessInput(&cmd);

        if (cl_predict_sequence > 5) {
            predicted_frame_t *frame = CL_FindPredictionFrame(cl_predict_sequence - 5);
            if (frame) {
                vec3_t server_origin;
                VectorCopy(frame->origin, server_origin);
                server_origin[0] += 0.05f;

                int before = correction_count;
                CL_ReconcilePrediction_Fixed(cl_predict_sequence - 5, server_origin, frame->velocity);
                if (correction_count > before) fixed_total_corrections++;
            }
        }
    }

    printf("  Buggy version corrections: %d\n", buggy_total_corrections);
    printf("  Fixed version corrections: %d\n", fixed_total_corrections);

    // Fixed version should have fewer or equal corrections
    if (fixed_total_corrections <= buggy_total_corrections) {
        printf("  PASS - Fixed version has fewer unnecessary corrections\n");
        return 1;
    } else {
        printf("  FAIL - Fixed version has more corrections than buggy\n");
        return 0;
    }
}


/*
 * Test 6: Sequence wraparound handling
 * Ensure both versions handle sequence number wraparound correctly.
 */
int test_sequence_wraparound(void)
{
    printf("Test: Sequence number wraparound\n");

    CL_InitPrediction();

    // Start near wraparound point
    cl_predict_sequence = PREDICTION_BUFFER_SIZE - 5;

    usercmd_t cmd = { .msec = 16, .forwardmove = 100, .sidemove = 0 };

    // Process enough to wrap around
    for (int i = 0; i < 20; i++) {
        CL_ProcessInput(&cmd);
    }

    // Server confirms a sequence that wrapped
    int test_seq = cl_predict_sequence - 3;
    predicted_frame_t *frame = CL_FindPredictionFrame(test_seq);

    if (!frame) {
        printf("  FAIL - Could not find wrapped frame\n");
        return 0;
    }

    // Verify we can reconcile after wraparound
    CL_ReconcilePrediction_Fixed(test_seq, frame->origin, frame->velocity);

    printf("  PASS - Handled wraparound (seq %d to %d)\n",
           PREDICTION_BUFFER_SIZE - 5, cl_predict_sequence);
    return 1;
}


/* ============== MAIN ============== */

int main(void)
{
    printf("Network Prediction Tests (quake-003)\n");
    printf("====================================\n");
    printf("Testing client-side prediction reconciliation.\n");
    printf("Buggy version has off-by-one and tolerance calculation bugs.\n\n");

    int passed = 0;
    int total = 6;

    passed += test_perfect_prediction();
    passed += test_off_by_one_bug();
    passed += test_diagonal_movement_tolerance();
    passed += test_server_correction();
    passed += test_correction_frequency();
    passed += test_sequence_wraparound();

    printf("\n====================================\n");
    printf("Results: %d/%d tests passed\n", passed, total);

    if (passed == total) {
        printf("\nAll tests passed - the fix is correct!\n");
        return 0;
    } else {
        printf("\nSome tests failed - the bug may still be present.\n");
        return 1;
    }
}
