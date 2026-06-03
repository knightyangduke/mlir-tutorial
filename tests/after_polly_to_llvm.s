	.file	"LLVMDialectModule"
	.text
	.globl	test_poly_fn                    # -- Begin function test_poly_fn
	.p2align	4
	.type	test_poly_fn,@function
test_poly_fn:                           # @test_poly_fn
	.cfi_startproc
# %bb.0:
	pushq	%rbp
	.cfi_def_cfa_offset 16
	pushq	%r15
	.cfi_def_cfa_offset 24
	pushq	%r14
	.cfi_def_cfa_offset 32
	pushq	%r13
	.cfi_def_cfa_offset 40
	pushq	%r12
	.cfi_def_cfa_offset 48
	pushq	%rbx
	.cfi_def_cfa_offset 56
	pushq	%rax
	.cfi_def_cfa_offset 64
	.cfi_offset %rbx, -56
	.cfi_offset %r12, -48
	.cfi_offset %r13, -40
	.cfi_offset %r14, -32
	.cfi_offset %r15, -24
	.cfi_offset %rbp, -16
	movl	%edi, %ebx
	movl	$104, %edi
	callq	malloc@PLT
	movq	%rax, %r14
	movq	%rax, %r13
	addq	$63, %r13
	andq	$-64, %r13
	xorl	%eax, %eax
	cmpq	$9, %rax
	jg	.LBB0_3
	.p2align	4
.LBB0_2:                                # =>This Inner Loop Header: Depth=1
	movl	%ebx, (%r13,%rax,4)
	incq	%rax
	cmpq	$9, %rax
	jle	.LBB0_2
.LBB0_3:
	movl	$104, %edi
	callq	malloc@PLT
	movq	%rax, %r15
	movq	%rax, %rbp
	addq	$63, %rbp
	andq	$-64, %rbp
	xorl	%eax, %eax
	cmpq	$9, %rax
	jg	.LBB0_6
	.p2align	4
.LBB0_5:                                # =>This Inner Loop Header: Depth=1
	movl	$0, (%rbp,%rax,4)
	incq	%rax
	cmpq	$9, %rax
	jle	.LBB0_5
.LBB0_6:
	movabsq	$12884901890, %rax              # imm = 0x300000002
	movq	%rax, (%rbp)
	movl	$4, 8(%rbp)
	xorl	%eax, %eax
	cmpq	$9, %rax
	jg	.LBB0_9
	.p2align	4
.LBB0_8:                                # =>This Inner Loop Header: Depth=1
	movl	(%r13,%rax,4), %ecx
	addl	%ecx, (%rbp,%rax,4)
	incq	%rax
	cmpq	$9, %rax
	jle	.LBB0_8
.LBB0_9:
	movl	$104, %edi
	callq	malloc@PLT
	movq	%rax, %r12
	movq	%rax, %rsi
	addq	$63, %rsi
	andq	$-64, %rsi
	movq	$0, (%rsi)
	movq	$0, 8(%rsi)
	movq	$0, 16(%rsi)
	movq	$0, 24(%rsi)
	movq	$0, 32(%rsi)
	xorl	%edi, %edi
	movabsq	$-3689348814741910323, %r8      # imm = 0xCCCCCCCCCCCCCCCD
	movq	%rsi, %r9
	jmp	.LBB0_10
	.p2align	4
.LBB0_14:                               #   in Loop: Header=BB0_10 Depth=1
	incq	%rdi
	addq	$4, %r9
.LBB0_10:                               # =>This Loop Header: Depth=1
                                        #     Child Loop BB0_12 Depth 2
	cmpq	$9, %rdi
	jg	.LBB0_15
# %bb.11:                               #   in Loop: Header=BB0_10 Depth=1
	movq	%rdi, %rcx
	xorl	%r10d, %r10d
	.p2align	4
.LBB0_12:                               #   Parent Loop BB0_10 Depth=1
                                        # =>  This Inner Loop Header: Depth=2
	movq	%rcx, %rax
	mulq	%r8
	cmpq	$9, %r10
	jg	.LBB0_14
# %bb.13:                               #   in Loop: Header=BB0_12 Depth=2
	andq	$-8, %rdx
	leaq	(%rdx,%rdx,4), %rax
	movq	%r9, %rdx
	subq	%rax, %rdx
	movl	(%rbp,%rdi,4), %eax
	imull	(%rbp,%r10,4), %eax
	addl	%eax, (%rdx,%r10,4)
	incq	%r10
	incq	%rcx
	jmp	.LBB0_12
.LBB0_15:
	xorl	%eax, %eax
	cmpq	$9, %rax
	jg	.LBB0_18
	.p2align	4
.LBB0_17:                               # =>This Inner Loop Header: Depth=1
	movl	(%r13,%rax,4), %ecx
	subl	%ecx, (%rsi,%rax,4)
	incq	%rax
	cmpq	$9, %rax
	jle	.LBB0_17
.LBB0_18:
	addq	$36, %rsi
	xorl	%ebp, %ebp
	movl	$1, %eax
	cmpq	$10, %rax
	jg	.LBB0_21
	.p2align	4
.LBB0_20:                               # =>This Inner Loop Header: Depth=1
	imull	%ebx, %ebp
	addl	(%rsi), %ebp
	incq	%rax
	addq	$-4, %rsi
	cmpq	$10, %rax
	jle	.LBB0_20
.LBB0_21:
	movq	%r14, %rdi
	callq	free@PLT
	movq	%r15, %rdi
	callq	free@PLT
	movq	%r12, %rdi
	callq	free@PLT
	movl	%ebp, %eax
	addq	$8, %rsp
	.cfi_def_cfa_offset 56
	popq	%rbx
	.cfi_def_cfa_offset 48
	popq	%r12
	.cfi_def_cfa_offset 40
	popq	%r13
	.cfi_def_cfa_offset 32
	popq	%r14
	.cfi_def_cfa_offset 24
	popq	%r15
	.cfi_def_cfa_offset 16
	popq	%rbp
	.cfi_def_cfa_offset 8
	retq
.Lfunc_end0:
	.size	test_poly_fn, .Lfunc_end0-test_poly_fn
	.cfi_endproc
                                        # -- End function
	.type	.L__constant_3xi32,@object      # @__constant_3xi32
	.section	.rodata,"a",@progbits
	.p2align	6, 0x0
.L__constant_3xi32:
	.long	2                               # 0x2
	.long	3                               # 0x3
	.long	4                               # 0x4
	.size	.L__constant_3xi32, 12

	.type	.L__constant_10xi32,@object     # @__constant_10xi32
	.p2align	6, 0x0
.L__constant_10xi32:
	.zero	40
	.size	.L__constant_10xi32, 40

	.section	".note.GNU-stack","",@progbits
