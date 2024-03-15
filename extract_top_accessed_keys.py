import os, sys

if len(sys.argv) != 2:
    exit(0)
filename = sys.argv[1]
f = open(filename, 'r')
key2freq = dict()
for line in f:
    tmp = line.strip().split(' ')
    if tmp[1] in key2freq:
        key2freq[tmp[1]] += 1
    else:
        key2freq[tmp[1]] = 1

key2freq_list = []
for k in key2freq:
    key2freq_list.append([key2freq[k], k])

key2freq_list.sort()
key2freq_list.reverse()

if len(key2freq_list) > 40:
    print(key2freq_list[:40])
else:
    print(key2freq_list)

