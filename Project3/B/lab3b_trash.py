# NAME: Natasha Sarkar
# EMAIL: nat41575@gmail.com
# ID: 904743795

# DEBUGGED & TESTED
# nothing yet...

# NEED TO TEST & DEBUG: 
# INVALID BLOCKS
# UNREFERENCED BLOCKS
# DUPLICATE BLOCKS
# ALLOCATED BLOCKS ON FREE LIST
# RESERVED BLOCKS
# ALLOCATED INODE ON FREE LIST
# UNALLOCATED INODE NOT ON FREE LIST
# LINKS VS LINK COUNT 
# INVALID INODES
# UNALLOCATED INODES

# NEED TO WRITE:
# DIRECTORY LINKS

import sys # for opening the argument file

# key: block number
# value: [inode number, offset, level]
block_dict = {} 

# key: inode number
# value: 'allocated' or 'unallocated'
inode_dict_alloc = {}

# key: inode number
# value: link count
inode_dict_lc = {} 

# key: inode number
# value: dict of referenced links {directory name:parent inode}
inode_dict_lr = {} 

# set of free blocks
bfree = set()

# set of free inodes
ifree = set()

# set of reserved blocks
reserved_blocks = set([0, 1, 2, 3, 4])

# stores information about duplicate blocks
# key: block number
# value: [ [inode number1, offset1, level1] 
#		   [inode number2, offset2, level2]... ]
# this dictionary does not include the first reference to the block
dup_blocks = {} 

try:
	input_file = open(sys.argv[1], "r")
except:
	print('file does not exist')
	exit()

lines = input_file.readlines()

for i in lines:
	entry = i.split(",")
	summary_type = entry[0]

	if summary_type == 'SUPERBLOCK': # get basic information
		total_blocks = int(entry[1])
		total_inodes = int(entry[2])

	elif summary_type == 'BFREE': # put in free blocks list
		bfree.add(int(entry[1])) 

	elif summary_type == 'IFREE': # put in free inodes list
		ifree.add(int(entry[1])) 


	elif summary_type == 'INODE':
		inode_num = int(entry[1])

		for i in range(12, 27):
			block_num = int(entry[i])
			if block_num < 0 or block_num > total_blocks: # check validity
				print('INVALID BLOCK ' + block_num + ' IN INODE ' + inode_num + ' AT OFFSET 0')
				continue
			elif block_num in reserved_blocks and block_num != 0: # block is reserved
				print('RESERVED BLOCK ' + block_num + ' IN INODE ' + inode_num + ' AT OFFSET 0')
				continue
			elif block_num == 0:
				continue
			elif block_num not in block_dict: # put in block dict {block:[inode, offset, level]}
				block_dict[block_num] = [inode_num, 0, 0]
			elif block_num not in dup_blocks: # 2nd reference to block (duplicate)
				dup_blocks[block_num] = [ [inode_num, 0, 0] ]
			else: # third or more reference to block (duplicate)
				dup_blocks[block_num].add([inode_num, 0, 0])

		# put in inode dict (link count) {inode number:link count}
		inode_dict_lc[inode_num] = int(entry[6])

		# put in inde dict (un/allocated) {inode number:un/allocated}
		if entry[2] == 0: # unallocated inode
			inode_dict_alloc[inode_num] = "unallocated"
		else: # allocated inode
			inode_dict_alloc[inode_num] = "allocated"


	elif summary_type == 'INDIRECT':
		block_num = entry[5]
		inode_num = entry[1]

		level = entry[2]
		if level == 1:
			strlvl = "INDIRECT"
			offset = 12
		elif level == 2:
			strlvl = "DOUBLE INDIRECT"
			offset = 268
		elif level == 3:
			strlvl = "TRIPLE INDIRECT"
			offset = 65804

		if block_num < 0 or block_num > total_blocks: # check validity
			print('INVALID ' + strlvl + ' BLOCK ' + block_num + ' IN INODE ' + inode_num + ' AT OFFSET ' + offset)
		elif block_num in reserved_blocks:
			print('RESERVED ' + strlvl + ' BLOCK ' + block_num + ' IN INODE ' + inode_num + ' AT OFFSET ' + offset)
		elif block_num not in block_dict: # put in block dict {block:[inode, offset, level]}
			block_dict[block_num] = [inode_num, offset, level]
		elif block_num not in dup_blocks: # 2nd reference to block (duplicate)
			dup_blocks[block_num] = [ [inode_num, offset, level] ]
		else: # 3rd or more reference to block (duplicate)
			dup_blocks[block_num].add([inode_num, offset, level])


	elif summary_type == 'DIRENT': # put in inode dict (link ref)
		inode_num = entry[3]
		dir_name = entry[6]
		parent_inode = entry[1]

		if inode_num < 1 or inode_num > total_inodes:
			print('DIRECTORY INODE ' + parent_inode + ' NAME ' + dir_name + ' INVALID INODE ' + inode_num)

		if inode_num not in inode_dict_lr: # inode not in dictionary yet, add it
			inode_dict_lr[inode_num] = dict()
		inode_dict_lr[inode_num][dir_name] = parent_inode # add {dir name:entry num}


# first find basic information from superblock
# go through lines and create a set of all bfrees
# then make a set of all allocated block numbers: inode, indirect
	# if there is an unreferenced block, print UNREFERENCED BLOCK
	# for [from first block to last block, based on info from superblock]
		# if cannot find block in either set, then there is an error
for x in range(1, total_blocks + 1):
	if x not in bfree and x not in block_dict:
		print('UNREFERENCED BLOCK ' + x)
	elif x in bfree and x in block_dict:
		print('ALLOCATED BLOCK ' + x ' ON FREELIST')


# compare list of allocated/unallocated inodes with the free inode bitmaps
for x in range(1, total_inodes + 1):
	if (x not in ifree and x not in inode_dict_alloc) or (x in inode_dict_alloc and inode_dict_alloc[x] == "unallocated"):
		print('UNALLOCATED INODE ' + x + ' NOT ON FREELIST')
	elif (x in inode_dict_alloc and inode_dict_alloc[x] == "allocated" and x in ifree):
		print('ALLOCATED INODE ' + x + ' ON FREELIST')


# print out duplicate block entries
for x in dup_blocks:
	print('DUPLICATE BLOCK ' + x + ' IN INODE ' + block_dict[x][0] + ' AT OFFSET 0')

	for y in dup_blocks[x]:

		level = y[2]
		if level == 1:
			strlvl = "INDIRECT"
		elif level == 2:
			strlvl = "DOUBLE INDIRECT"
		elif level == 3:
			strlvl = "TRIPLE INDIRECT"

		print('DUPLICATE ' + strlvl + ' BLOCK ' + x + ' IN INODE ' + y[0] ' AT OFFSET ' + y[1])


# number of directory entries for each inode should equal its link count
for x in inode_dict_lc:
	linkcount = inode_dict_lc[x]
	links = len(inode_dict_lr[x])
	if linkcount != links:
		print('INODE ' + x + ' HAS ' + links + ' LINKS BUT LINKCOUNT IS ' + linkcount)

# find unallocated inodes from directory entries
for x in inode_dict_lr:
	if x in ifree or x not in inode_dict_lc:
		for y in inode_dict_lc[x]: # dictionary {dir_name:parent_inode}
			print('DIRECTORY INODE ' + inode_dict_lc[x][y] + ' NAME ' + y + ' UNALLOCATED INODE ' + x)
		