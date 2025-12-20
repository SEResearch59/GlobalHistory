import os, sys

if __name__ == '__main__':
    atks = os.listdir('.')
    ecdsa_dir = './Modules/ECDSA/' # check it before use
    keys = os.listdir(ecdsa_dir)
    atk_res = []
    key_res = []
    for str in atks:
        if str.find('amd_attack_ecdsa') != -1 and str.endswith('.txt'):
            atk_res.append(str)
    for str in keys:
        if str.find('amd_attack_ecdsa') != -1 and str.endswith('.txt'):
            key_res.append(ecdsa_dir+str)
    # assert(key_res.__len__() != atk_res.__len__(), f'atk res ({atk_res.__len__()}) and key res ({key_res.__len__()}) length not same')
    correct_0 = 0
    correct_1 = 0
    total = 0
    for i in range(atk_res.__len__()):
        with open(atk_res[i], 'r') as fd:
            lines = fd.readlines()
            fd_key = open(key_res[i], 'r')
            key_lines = fd_key.readlines()
            lineNr = 0
            lineNrk = 0
            while lineNrk < key_lines.__len__() and lineNr < lines.__len__():
                res0 = int(lines[lineNr].strip()[-1])
                lineNr += 1
                res1 = int(lines[lineNr].strip()[-1])
                lineNr += 1
                key_first = key_lines[lineNrk].strip()[0]
                lineNrk += 1
                # print(key_first)
                key_first_value = int(key_first, base=16) >> 2
                if res0 == (key_first_value >> 1):
                    correct_0 += 1
                    if res1 == (key_first_value & 1):
                        correct_1 += 1
                total += 1
            fd_key.close()
    print(f'total: {total}, res0 correct: {correct_0}, res1 correct: {correct_1}\n')
