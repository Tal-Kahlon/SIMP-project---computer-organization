.word 256 821
.word 257 26421
.word 258 26520
add $t0,$zero,$imm,1 #$t0=1
add $t2,$zero,$imm,1 #$t2 =1 
sll $sp,$t0,$imm,11 #$t0=1<<11 =2048 
add $sp,$sp,$imm,-3 #open 3 spots in stack
sw $s0,$sp,$zero,0 # store $s0
sw $s1,$sp,$imm,1 # store $s1
sw $s2,$sp,$imm,2 # store $s2
lw $a0,$zero,$imm,256 #$a0=A
lw $a1,$zero,$imm,257 #$a1=B
lw $a2,$zero,$imm,258 #$a2=C
sub $t0,$a2,$a1,0 #$t0 = C-B number of columns 
add $t0,$t0,$imm,1 #$t0 = C-B number of columns +1
sub $t1,$a1,$a0,0 #$t1 = B-A 
sra $t1,$t1,$imm,8 #$t1 = (B-A)/256 number of rows
add $t1,$t1,$imm,1 #$t1 = B-A +1
add $a3,$t1,$zero,0 #save B-A+1 into a3
out $a0,$zero,$imm,20 #monitoraddr = A 
add $s1,$zero,$imm,255 #$s1 =255 - white 
out $s1,$zero,$imm,21 # monitordata =255 - white pixel
out $t2,$zero,$imm,22 # monitorcmd = 1 
add $s2,$zero,$a1,0 #$s2 =B
printfinal:
bgt $imm,$s2,$a2,start #if B>C go to first loop  
out $s2,$zero,$imm,20 # monitoraddr = $s2
add $s2,$s2,$imm,1 # $s2 ++ (B++)
out $t2,$zero,$imm,22 # monitorcmd = 1 
beq $imm,$zero,$zero,printfinal # go back to final row print loop
start:
add $s1,$zero,$zero,0 # $s1=0 counter = 0
add $s0,$zero,$t0,0 # $s0 = number of columns x=number of columns 
while:
ble $imm,$s0,$zero,if1 #if x<=0 go to if1 
sub $s0,$s0,$t1,0 # x=x-(row index)
add $s1,$s1,$imm,1 # counter ++
beq $imm,$zero,$zero,while # go back to while
if1:
bne $imm,$s0,$zero,start2 # if x!=0 go to second for loop
add $s1,$s1,$imm,1 # counter ++
start2:
add $s0,$zero,$zero,0 #s0 = counter2 
for1:
ble $imm,$t1,$t2,end #if row index<=1 go to end 
add $s2,$zero,$zero,0 #$s2 =0 -index for second loop
add $s0,$s0,$s1,0 # counter2 = counter2 + counter
for2:
bgt $imm,$s2,$s0,back # if loop index >cunter go back to first loop
bgt $imm,$s2,$t0,back # if pixel index > C stop and go back to write next line
sub $a1,$a3,$t1,0 # number of row - index row
add $a1,$a1,$imm,1 #number of row - index row-1
sll $a1,$a1,$imm,8 # $a1 = (number of row - index row-1)*256
add $a1,$a1,$a0,0 #$a1+A 
add $a1,$s2,$a1,0# $a1 = row_number*256 +A+loop_index
out $a1,$zero,$imm,20 #monitoraddr = $a1
out $t2,$zero,$imm,22 # monitorcmd = 1 
add $s2,$s2,$imm,1 # loop_index ++
beq $imm,$zero,$zero,for2 # go back to for2 
back:
add $t1,$t1,$imm,-1 #row index--
beq $imm,$zero,$zero,for1 # go back to first for loop 
end:
lw $s0,$sp,$zero,0 # restore $s0
lw $s1,$sp,$imm,1 # restore $s1
lw $s2,$sp,$imm,2 # restore $s2
add $sp,$zero,$imm,3 #close stack
halt $zero,$zero,$zero,0 #halt 