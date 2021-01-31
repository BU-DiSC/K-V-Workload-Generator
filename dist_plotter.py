import csv
import matplotlib.pylab as plt
import sys

number_of_digits_to_use = 2
filepath = "x_workload.txt"

def read_from_disk(filepath):
    results = list()
    with open(filepath, 'r') as csvfile: 
        # creating a csv reader object 
        csvreader = csv.reader(csvfile) 
        # extracting each data row one by one 
        for row in csvreader:
            key = str(row).split()[1].replace(",", "").replace("]", "").replace("'", "")
            diff = 10 - len(key)
            for i in range(diff):
                key = "0" + key
              	# print(key)

                # print(diff)
            results.append(key[:number_of_digits_to_use])
    # print(results)
    return results

def count_elements(seq):
    hist = {}
    for i in seq:
        try:
            hist[i].append("+")
        except KeyError:
            hist[i] = ["+"]
    # print(hist)
    return hist


counted = count_elements(read_from_disk(filepath))
counted = sorted(counted.items(), key=lambda s: s[0])
# print(counted)
values = list()
names = list()
for i in counted:
    values.append(len(str(i[1]).replace(",", "").replace("[", "").replace("]", "").replace("'", "").replace(" ", "")))

for j in counted:
    names.append(str(j[0]))

# print(values)
# print(names)

plt.bar(names, values)
plt.show()
