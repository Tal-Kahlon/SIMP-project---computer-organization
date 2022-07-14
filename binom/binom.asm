	.word 256 5
	.word 257 2
	add $t1,$zero,$imm,1			 #$t1 = 1
	sll $sp,$t1,$imm,11					#$sp = 1<<11 = 2048
	lw $a0,$zero,$imm,0x100				#load $a0 = n
	lw $a1,$zero,$imm,0x101				 #load $a1 = k
	jal $ra,$imm, $zero,rec_binom		#calac r$v0 = rec_binom(n,k)
	sw $v0, $zero, $imm, 258			# store rec_binom in 257
	halt $zero, $zero, $zero, 0			# halt
rec_binom:
	add $sp,$sp,$imm,-4					#open space in stack
	sw $a0,$sp,$imm,0					#store $a0 = n 
	sw $a1,$sp,$imm,1					#store $a1 = k
	sw $s0,$sp,$imm,2					#store $so
	sw $ra,$sp,$imm,3					 #store ra
	sub $t2,$a0,$a1,0					#t2 = n-k
	mul $t2,$t2,$a1,0					# t2 = 0 one of the condition is happend
	bne $imm,$t2,$zero,l1				 #if(k==0|| k==n)  not work should go to else 
	add $v0,$imm,$zero,1				# return 1
	beq $imm, $zero, $zero, l2
l1:
	add $a0,$a0,$imm,-1					#n = n-1
	jal $ra, $imm,$zero,rec_binom		#cala $v0=rec_binom(n-1,k)
	add $s0,$v0,$zero,0					# $s0 = rec_binom(n-1,k)
	lw $a0,$sp,$imm,0					#restore n =$a0
	add $a0,$a0,$imm,-1					#n = n-2
	add $a1,$a1,$imm,-1					#k = k-1
	jal $ra, $imm,$zero,rec_binom		#cala $v0=rec_binom(n-1,k-1)
	add $v0,$v0,$s0,0					 #v0 = binom(n-1,k-1) + binom(n-1,k)
	lw $a1,$sp,$imm ,1					#restore $a1=k
	lw $ra ,$sp,$imm,3					#restore $ra 
	lw $s0,$sp,$imm,2					 #restore $s0
	lw $a0,$sp,$imm,0	
l2:
	add $sp, $sp, $imm, 4				# pop 3 items from stack
	beq $ra, $zero, $zero, 0			# and return