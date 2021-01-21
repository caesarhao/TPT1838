#!/usr/bin/env python3
'''
script to analyze BCT file
'''
import os
import sys
from collections import namedtuple
import struct
import numpy as np

bctType = np.dtype([
	('crypto_hash', '<16B'),
	('random_aes_blk', '<16B'),
	('boot_data_version', '<1L'),
	('block_size_log2', '<1L'),
	('page_size_log2', '<1L'),
	('partition_size', '<1L'),
	('num_param_sets', '<1L'),
	('dev_type', '<1L')
)]
devType = np.dtype([
	
)]

def main():
	pass

if __name__ == "__main__":
	main()
	
