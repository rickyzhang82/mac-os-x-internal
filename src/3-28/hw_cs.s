// hw_cs.s
//
// hw_compare_and_store(u_int32_t old,
//                      u_int32_t new,
//                      u_int32_t *address,
//                      u_int32_t *dummyaddress)
//
// Performs the following atomically:
//
// Compares old value to the one at address, and if they are equal, stores new
// value, returning true (1). On store failure, returns false (0). dummyaddress
// points to a valid, trashable u_int32_t location, which is written to for
// canceling the reservation in case of a failure.
   
        .align  5
        .globl  _hw_compare_and_store
   
_hw_compare_and_store:
        // Arguments:
        //      r3      old
        //      r4      new
        //      r5      address
        //      r6      dummyaddress
   
        // Save the old value to a free register.
        mr      r7,r3
   
looptry:
   
        // Retrieve current value at address.
        // A reservation will also be created.
        lwarx   r9,0,r5
   
        // Set return value to true, hoping we will succeed.
        li      r3,1
   
        // Do old value and current value at address match?
        cmplw   cr0,r9,r7
   
        // No! Somebody changed the value at address.
        bne--   fail
   
        // Try to store the new value at address.
        stwcx.  r4,0,r5
   
        // Failed! Reservation was lost for some reason.
        // Try again.
        bne--   looptry
   
        // If we use hw_compare_and_store to patch/instrument code dynamically,
        // without stopping already running code, the first instruction in the
        // newly created code must be isync. isync will prevent the execution
        // of instructions following itself until all preceding instructions
        // have completed, discarding prefetched instructions. Thus, execution
        // will be consistent with the newly created code. An instruction cache
        // miss will occur when fetching our instruction, resulting in fetching
        // of the modified instruction from storage.
        isync
   
        // return
        blr
   
fail:
        // We want to execute a stwcx. that specifies a dummy writable aligned
        // location. This will "clean up" (kill) the outstanding reservation.
        mr      r3,r6
        stwcx.  r3,0,r3
   
        // set return value to false.
        li      r3,0
   
        // return
        blr
