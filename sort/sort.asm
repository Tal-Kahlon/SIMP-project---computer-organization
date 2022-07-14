	.word 256 100
	.word 257 99
	.word 258 98
	.word 259 97
	.word 260 96
	.word 261 95
	.word 262 94
	.word 263 93
	.word 264 92
	.word 265 91
	.word 266 90
	.word 267 89
	.word 268 88
	.word 269 87
	.word 270 86
	.word 271 85
	add $t1,$zero,$imm,1 #$t1 = 1
	sll $sp,$t1,$imm,11 #$sp = 1<<11 = 2048
	add $sp,$sp,$imm,-5 #open space in stack
	sw $s0,$sp,$zero,0 #store $s0
	sw $s1,$sp,$imm,1 #stre $s1
	sw $s2,$sp,$imm,2 #store $s2
	sw $a0,$sp,$imm,3 #store $a0
	sw $a1,$sp,$imm,4 #store $a1
	add $t0,$zero,$imm, 0 #temp=0
	add $t1,$zero,$imm,0 #i=0
	add $t2,$zero,$imm,1 #j=1
	add $a0,$zero,$imm,14 #$a0=14; אולי לא צריך
	add $a1,$zero,$imm,15 #$a1=15;
for1:
	bgt $imm,$t1,$a1,end # if i>15 finished sorting
for2:
	bgt $imm,$t2,$a1,endofj #if j>15 i++ and go back to first loop
	lw $s0,$t1,$imm,256 #$s0 = list[i]
	lw $s1,$t2,$imm,256 #$s1 = list[j]
	ble $imm,$s0,$s1,forj #if list[i]<=list[j] j++ and again to second loop
	add $t0,$s0,$zero,0 #temp = arr[i]
	sw $s1,$t1,$imm,256 #arr[i] = arr[j]
	sw $t0,$t2,$imm,256 #arr[j]=temp
	beq $imm,$zero,$zero,forj # j++ and go back to for2
endofj:
	add $t1,$t1,$imm,1 #i++
	add $t2,$t1,$imm,1 #j =i+1
	beq $imm,$zero,$zero,for1 # jump to loop 
forj:
	add $t2,$t2,$imm,1 #j++
	beq $imm,$zero,$zero,for2 # jump to loop 
end:
	lw $s0,$sp,$zero,0 #restore $s0
	lw $s1,$sp,$imm,1 #restore $s1
	lw $s2,$sp,$imm,2 #restore $s2
	lw $a0,$sp,$imm,3 #restore $a0
	lw $a1,$sp,$imm,4 #restore $a1
	add $sp,$zero,$imm,5 #close space in stack
	halt $zero, $zero, $zero, 0			# halt
