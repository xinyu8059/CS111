# NAME: Natasha Sarkar
# EMAIL: nat41575@gmail.com
# ID: 904743795

# POSSIBLE INCONSISTENCIES:

# UNREFERENCED BLOCKS
# DUPLICATE BLOCKS
# ALLOCATED BLOCKS ON FREE LIST
# RESERVED BLOCKS
# INVALID BLOCKS
# ALLOCATED INODE ON FREE LIST
# UNALLOCATED INODE NOT ON FREE LIST
# DIRECTORY LINKS
# LINKS VS LINK COUNT 
# INVALID INODES IN DIRECTORY ENTRIES
# UNALLOCATED INODES IN DIRECTORY ENTRIES

import sys # for opening the argument file

# key: block number
# value: [ [inode number, offset, level] ]
block_dict = {} 

# key: inode number
# value: link count
inode_dict_lc = {} 

# key: inode number
# value: ref link count
inode_dict_reflc = {}

# key: inode number
# value: parent inode
inode_dict_lr = {}

# key: parent inode number
# value: inode number
# stores only '..' directories
inode_dict_parents = {}

# referenced inodes
# key: inode number
# value: diredctory name
ref_inode = {}

# set of free blocks
bfree = set()

# set of free inodes
ifree = set()


# set of reserved blocks
reserved_blocks = set([0, 1, 2, 3, 4, 5, 6, 7, 64])

total_blocks = 0
total_inodes = 0
offset = 0
linkcount = 0
links = 0
damaged = False

# open the file from the command line argument
try:
	input_file = open(sys.argv[1], "r")
except:
	sys.stderr.write('file does not exist\n')
	exit(1)

lines = input_file.readlines()
# parse the input file
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
		# put in inode dict (link count) {inode number:link count}
		inode_dict_lc[inode_num] = int(entry[6])
		for i in range(12, 27): # block addresses
			block_num = int(entry[i])
			if block_num == 0: # unused block address
				continue

			if i == 24:
				strlvl = " INDIRECT"
				offset = 12
				level = 1
			elif i == 25:
				strlvl = " DOUBLE INDIRECT"
				offset = 268
				level = 2
			elif i == 26:
				strlvl = " TRIPLE INDIRECT"
				offset = 65804
				level = 3
			else:
				strlvl = ""
				offset = 0
				level = 0

			if block_num < 0 or block_num > total_blocks: # check validity
				print('INVALID' + strlvl + ' BLOCK ' + str(block_num) + ' IN INODE ' + str(inode_num) + ' AT OFFSET ' + str(offset))
				damaged = True
			elif block_num in reserved_blocks and block_num != 0: # block is reserved
				print('RESERVED' + strlvl + ' BLOCK ' + str(block_num) + ' IN INODE ' + str(inode_num) + ' AT OFFSET ' + str(offset))
				damaged = True
			elif block_num not in block_dict: # 1st reference to block
				block_dict[block_num] = [ [inode_num, offset, level] ]
			else: # 2nd or more reference to block (duplicate)
				block_dict[block_num].append([inode_num, offset, level])


	elif summary_type == 'INDIRECT':
		block_num = int(entry[5])
		inode_num = int(entry[1])

		level = int(entry[2])
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
			print('INVALID ' + strlvl + ' BLOCK ' + str(block_num) + ' IN INODE ' + str(inode_num) + ' AT OFFSET ' + str(offset))
			damaged = True
		elif block_num in reserved_blocks:
			print('RESERVED ' + strlvl + ' BLOCK ' + str(block_num) + ' IN INODE ' + str(inode_num)+ ' AT OFFSET ' + str(offset))
			damaged = True
		elif block_num not in block_dict: # 1st reference to block
			block_dict[block_num] = [ [inode_num, offset, level] ]
		else: # 2nd or more reference to block (duplicate)
			block_dict[block_num].append([inode_num, offset, level])


	elif summary_type == 'DIRENT': # put in inode dict (link ref)
		dir_name = entry[6]
		parent_inode = int(entry[1])
		inode_num = int(entry[3])

		#print("adding" + str(inode_num))
		ref_inode[inode_num] = dir_name

		if inode_num not in inode_dict_reflc:
			inode_dict_reflc[inode_num] = 1
		else:
			inode_dict_reflc[inode_num] = inode_dict_reflc[inode_num] + 1

		if inode_num < 1 or inode_num > total_inodes:
			print('DIRECTORY INODE ' + str(parent_inode) + ' NAME ' + dir_name[0:len(dir_name)- 2] + "' INVALID INODE " + str(inode_num))
			damaged = True
			continue

		if dir_name[0:3] == "'.'" and parent_inode != inode_num:
			print('DIRECTORY INODE ' + str(parent_inode) + " NAME '.' LINK TO INODE " + str(inode_num) + ' SHOULD BE ' + str(parent_inode))
			damaged = True

		elif dir_name[0:3] == "'.'":
			continue

		elif dir_name[0:4] == "'..'":
			inode_dict_parents[parent_inode] = inode_num 

		else:
			inode_dict_lr[inode_num] = parent_inode


# unreferenced blocks & allocated blocks on freelist 
for x in range(1, total_blocks + 1):
	if x not in bfree and x not in block_dict and x not in reserved_blocks:
		print('UNREFERENCED BLOCK ' + str(x))
		damaged = True
	elif x in bfree and x in block_dict:
		print('ALLOCATED BLOCK ' + str(x) + ' ON FREELIST')
		damaged = True


# compare list of allocated/unallocated inodes with the free inode bitmaps
for x in range(1, total_inodes + 1):
	if x not in ifree and x not in inode_dict_lc and x not in inode_dict_lr and x not in (1, 3, 4, 5, 6, 7, 8, 9, 10):
		print('UNALLOCATED INODE ' + str(x) + ' NOT ON FREELIST')
		damaged = True
	elif x in inode_dict_lc and x in ifree:
		print('ALLOCATED INODE ' + str(x) + ' ON FREELIST')
		damaged = True


# duplicate block entries
for block in block_dict:
	if len(block_dict[block]) > 1: # if the block has been referenced multiple times
		damaged = True
		for ref in block_dict[block]: # ref: [inode number, offset, level]
			level = int(ref[2])
			if level == 0:
				strlvl = ""
			elif level == 1:
				strlvl = " INDIRECT"
			elif level == 2:
				strlvl = " DOUBLE INDIRECT"
			elif level == 3:
				strlvl = " TRIPLE INDIRECT"

			# print out messages for all references
			print('DUPLICATE' + strlvl + ' BLOCK ' + str(block) + ' IN INODE ' + str(ref[0]) + ' AT OFFSET ' + str(ref[1]))


# directory links
for parent in inode_dict_parents:
	if parent == 2 and inode_dict_parents[parent] == 2:
		continue
	elif parent == 2:
		print("DIRECTORY INODE 2 NAME '..' LINK TO INODE " + str(inode_dict_parents[parent]) + " SHOULD BE 2")
		damaged = True
	elif parent not in inode_dict_lr:
		print("DIRECTORY INODE " + str(inode_dict_parents[parent]) + " NAME '..' LINK TO INODE " + str(parent) + " SHOULD BE " + str(inode_dict_parents[parent]))
		damaged = True
	elif inode_dict_parents[parent] != inode_dict_lr[parent]: 
		print("DIRECTORY INODE " + str(parent) + " NAME '..' LINK TO INODE " + str(parent) + " SHOULD BE " + str(inode_dict_lr[parent]))
		damaged = True


# links vs link count
for x in inode_dict_lc:
	linkcount = inode_dict_lc[x]

	if x in inode_dict_reflc:
		links = inode_dict_reflc[x]
	else:
		links = 0

	if links != linkcount:
		print('INODE ' + str(x) + ' HAS ' + str(links) + ' LINKS BUT LINKCOUNT IS ' + str(linkcount))
		damaged = True


# unallocated inodes 
for x in ref_inode:
	if x in ifree and x in inode_dict_lr:
		parent_inode = inode_dict_lr[x]
		dir_name = ref_inode[x]
		print('DIRECTORY INODE ' + str(parent_inode) + ' NAME ' + dir_name[0:len(dir_name)- 2] + "' UNALLOCATED INODE " + str(x))
		damaged = True


if damaged:
	exit(2)
else:
	exit(0)