	.text

	/*
         * save the old thread's registers,
         * restore the new thread's registers.
         */

	.globl thread_switch
thread_switch:
    sd  x1,  0(a0)
    sd  x2,  8(a0)
    sd  x8,  16(a0)
    sd  x9,  24(a0)
    sd  x18, 32(a0)
    sd  x19, 40(a0)
    sd  x20, 48(a0)
    sd  x21, 56(a0)
    sd  x22, 64(a0)
    sd  x23, 72(a0)
    sd  x24, 80(a0)
    sd  x25, 88(a0)
    sd  x26, 96(a0)
    sd  x27, 104(a0)

    ld  x1,  0(a1)
    ld  x2,  8(a1)
    ld  x8,  16(a1)
    ld  x9,  24(a1)
    ld  x18, 32(a1)
    ld  x19, 40(a1)
    ld  x20, 48(a1)
    ld  x21, 56(a1)
    ld  x22, 64(a1)
    ld  x23, 72(a1)
    ld  x24, 80(a1)
    ld  x25, 88(a1)
    ld  x26, 96(a1)
    ld  x27, 104(a1)

	ret    /* return to ra */
