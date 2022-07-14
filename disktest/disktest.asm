add $sp,$imm,$zero,3600 #$sp = 3600 stack pointer 
add $sp,$sp,$imm,-3 #open 3 spots in stack 
sw $s0,$zero,$sp,0 #store $s0
sw $s1,$imm,$sp,1 #store $s1
sw $s2,$imm,$sp,2 #store $s2
add $t0,$zero,$imm,1 #$t1 =1 
sll $t0,$t0,$imm,11 #$t0 = 2048
add $s1,$zero,$imm,8 # $s1 =8
add $a0,$zero,$zero,0 #$a0 = sector number
read:
bge $imm,$a0,$s1,start #if a0>=8 go to for1 
status:
in $t2,$zero,$imm,17 #$t2 = diskstatus 
bne $imm,$t2,$zero,status #if status not 0 try again
add $t2,$zero,$imm,1 #$t2=1 
out $t2,$zero,$imm,17 #$ diskstatus = 1  
out $a0,$zero,$imm,15 #disksector = $a0
out $t0,$zero,$imm,16 # diskbuffer address
add $s0,$zero,$imm,1 #$a0 =1
out $s0,$zero,$imm,14 #diskcmd = 1 - read 
add $t0,$t0,$imm,128 #$t0 +=128
add $a0,$a0,$imm,1 #$a0 = next sector
beq $imm,$zero,$zero,read # go back to read
start:
in $t2,$zero,$imm,17 #$t2 = diskstatus 
bne $imm,$t2,$zero,start #if status not 0 try again
add $t1,$zero,$imm,2048 #$t1=2048 first line of sector 0 
add $s2,$t1,$zero,0 # $s2 = 2048
add $t2,$zero,$imm,3072 #$t2 = 3072 first line of sector 8 in memin
add $a0,$zero,$zero,0 #$a0=0 i 
add $a2,$imm,$zero,128 #$a2=128
add $s0,$imm,$zero,8 #$s0 =8 
for1:
bge $imm,$a0,$a2,write #if i>=128 go to write 
add $a1,$zero,$zero,0 #$a1=0 j =0 
add $a3,$zero,$zero,0 #$a3 =0 
for2:
bge $imm,$a1,$s0,back #if j>=8 go back to first loop after updating index 
lw $s1,$s2,$zero,0 #$s1 = data of the line of relevant sector 
add $a3,$a3,$s1,0 #$a3 += $s1 
add $s2,$s2,$a2,0 # $s1 +=128 
add $a1,$a1,$imm,1 #j++
beq $imm,$zero,$zero,for2 # go back to second loop 
back:
sw $a3,$t2,$zero,0 #save to sector 8
add $t2,$t2,$imm,1 #go to next line in sector 8 
add $a0,$a0,$imm,1 #$i++
add $s2,$t1,$a0,0 # $s2 = next line in reading sectors 
beq $imm,$zero,$zero,for1 # go back to first loop
write:
in $t2,$zero,$imm,17 #$t2 = diskstatus 
bne $imm,$t2,$zero,write #if status not 0 try again
add $t2,$zero,$imm,1 #$t2=1 
out $t2,$zero,$imm,17 #$ diskstatus = 1  
add $a0,$zero,$imm,8 # $a0 = 8
out $a0,$zero,$imm,15 #disksector =8
add $t0,$imm,$zero,3072 #diskbuffer = 3072
out $t0,$zero,$imm,16 # diskbuffer update
add $a0,$zero,$imm,2 #$a0 =2
out $a0,$zero,$imm,14 #diskcmd = 2 - write
lw $s0,$zero,$sp,0 #restore $s0
lw $s1,$imm,$sp,1 #restore $s1
lw $s2,$imm,$sp,2 #restore $s2
add $sp,$sp,$imm,3 #close spots in stack
finish:
in $t2,$zero,$imm,17 #$t2 = diskstatus 
bne $imm,$t2,$zero,finish #if status not 0 try again
halt $zero,$zero,$zero,0 #halt