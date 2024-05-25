#if defined(__mips__) && _MIPS_SIM == _ABI64

#define CTZ_BUILD_ASM 1
#include <ctz/osfiber_asm_mips64.h>

// void ctz_fiber_swap(ctz_fiber_context* from, const ctz_fiber_context* to)
// a0: from
// v0: to
.text
.global CTZ_ASM_SYMBOL(ctz_fiber_swap)
.align 4
CTZ_ASM_SYMBOL(ctz_fiber_swap):

    // Save context 'from'

    // Store callee-preserved registers
    sd  $s0, CTZ_REG_s0($a0)
    sd  $s1, CTZ_REG_s1($a0)
    sd  $s2, CTZ_REG_s2($a0)
    sd  $s3, CTZ_REG_s3($a0)
    sd  $s4, CTZ_REG_s4($a0)
    sd  $s5, CTZ_REG_s5($a0)
    sd  $s6, CTZ_REG_s6($a0)
    sd  $s7, CTZ_REG_s7($a0)

    s.d  $f24, CTZ_REG_f24($a0)
    s.d  $f25, CTZ_REG_f25($a0)
    s.d  $f26, CTZ_REG_f26($a0)
    s.d  $f27, CTZ_REG_f27($a0)
    s.d  $f28, CTZ_REG_f28($a0)
    s.d  $f29, CTZ_REG_f29($a0)
    s.d  $f31, CTZ_REG_f30($a0)
    s.d  $f31, CTZ_REG_f31($a0)

    sd  $gp, CTZ_REG_gp($a0)
    sd  $sp, CTZ_REG_sp($a0)
    sd  $fp, CTZ_REG_fp($a0)
    sd  $ra, CTZ_REG_ra($a0)

    move  $v0, $a1 // Function have no return, so safe to touch v0

    // Recover callee-preserved registers
    ld  $s0, CTZ_REG_s0($v0)
    ld  $s1, CTZ_REG_s1($v0)
    ld  $s2, CTZ_REG_s2($v0)
    ld  $s3, CTZ_REG_s3($v0)
    ld  $s4, CTZ_REG_s4($v0)
    ld  $s5, CTZ_REG_s5($v0)
    ld  $s6, CTZ_REG_s6($v0)
    ld  $s7, CTZ_REG_s7($v0)

    l.d  $f24, CTZ_REG_f24($v0)
    l.d  $f25, CTZ_REG_f25($v0)
    l.d  $f26, CTZ_REG_f26($v0)
    l.d  $f27, CTZ_REG_f27($v0)
    l.d  $f28, CTZ_REG_f28($v0)
    l.d  $f29, CTZ_REG_f29($v0)
    l.d  $f31, CTZ_REG_f30($v0)
    l.d  $f31, CTZ_REG_f31($v0)

    ld  $gp, CTZ_REG_gp($v0)
    ld  $sp, CTZ_REG_sp($v0)
    ld  $fp, CTZ_REG_fp($v0)
    ld  $ra, CTZ_REG_ra($v0)

    // Recover arguments
    ld  $a0, CTZ_REG_a0($v0)
    ld  $a1, CTZ_REG_a1($v0)

    jr	$ra

#endif // defined(__mips__) && _MIPS_SIM == _ABI64
