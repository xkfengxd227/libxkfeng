#coding=utf-8
#!/usr/bin/bash python

import jieba
import jieba.analyse

wordfile = open('./data/split_chushi_zhishi.txt', 'w+')
for line in open('./data/chushi_zhishi.txt'):
	item = line.strip('\n\r')

	print item

	tags = jieba.analyse.extract_tags(item)

	tagsw = ",".join(tags)
	wordfile.write(tagsw)

wordfile.close()
