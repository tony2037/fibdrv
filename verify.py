expect = [0, 1]
result = []
result_split = []
dics = []

for i in range(2, 101):
    expect.append(expect[i - 1] + expect[i - 2])
with open('result.txt', 'r') as f:
    tmp = f.readline()
    while (tmp):
        result.append(tmp)
        tmp = f.readline()
    f.close()
for r in result:
    if (r.find('Reading') != -1):
        result_split.append(r.split(' '))
        k = int(result_split[-1][5].split(',')[0])
        f0 = int(result_split[-1][9])
        f1 = int(result_split[-1][-3].split('(')[-1])
        print('%s/%s/%s' %(k, f0, f1))
        dics.append((k, f0, f1))
for i in dics:
    f = i[1] + i[2] * (0xFFFFFFFFFFFFFFFF + 1)
    if (expect[i[0]] == f):
        print('f(%s) sucess' % str(i[0]))
    else:
        print('f(%s) fail' % str(i[0]))
        print(f)
        print(expect[i[0]])
        exit()
