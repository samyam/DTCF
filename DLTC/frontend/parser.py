import sys
from subprocess import call

#print sys.argv
f = open('input.txt','r')

# Z29(e1ae2am1am2a) -> Z29, ['e1a','e2a','m1a','m2a']
def parse_tensor(str,tsrnames,idxnames,tsrmap):
  x = str.split('(')
  name = x[0]
  inds = x[1][:-1]
  i = 0
  idx = ''
  idxs = []
  rngs = []
  for c in inds:
    i+=1
    idx+=c
    if i%3 == 0:
      idxs.append(idx)
      if idx[0]=='d' or idx[0]=='e' or idx[0]=='f':
        rngs.append('HOL')
      elif idx[0]=='m' or idx[0]=='n' or idx[0]=='l':
        rngs.append('PAR')
      else:
        print idx[0]
        print 'error'
        exit(0)
      idxnames.add(idx)
      idx = ''
#  print name, idxs
  tsrnames.add(name)
  tsrmap[name]=rngs
  return

exps = []

for line in f:
  print line
  if line[0]=='#': continue
  line = line.rstrip()
  line = line.split('+=')
  lhs = line[0]
  rhs = line[1].split('*')
  n = len(rhs)
  if (rhs[n-1][0]=='('):
    str = rhs[n-1][1:-1]
    coef = float(str)
    rhs.pop()
  else:
    coef = 1.0
  if len(rhs)==1:
    e = ['S',lhs,coef,rhs]
  elif len(rhs)==2:
    e = ['M',lhs,coef,rhs]
  else:
    print 'error'
    exit(0)
  exps.append(e)

tsrnames = set()
idxnames = set()
tsrmap = {}

#print len(exps)
for e in exps:
#  print e
  parse_tensor(e[1],tsrnames,idxnames,tsrmap)
  for s in e[3]:
    parse_tensor(s,tsrnames,idxnames,tsrmap)
idxnames = list(idxnames)
idxnames.sort()
tsrnames = list(tsrnames)
tsrnames.sort()

nocc = 0
nvrt = 0
for i in idxnames:
  if i[0]=='d' or i[0]=='e' or i[0]=='f':
    nocc += 1
  elif i[0]=='m' or i[0]=='n' or i[0]=='l':
    nvrt += 1
  else:
    print 'error'
    exit(0)

level = []

for e in exps:
  level.append(0)

check = ''
for e in exps:
  for f in e[3]:
    for g in exps:
#      print f,g[1]
      n1 = f.split('(')
      n2 = g[1].split('(')
#      print n1,n2
      if n1[0]==n2[0]:
        level[exps.index(e)]=max(level[exps.index(g)]+1,level[exps.index(e)])

exps2 = []
for e in exps:
  exps2.append([level[exps.index(e)],e])
exps2.sort()

def str2nmidx(str):
  x = str.split('(')
  name = x[0]
  inds = x[1][:-1]
  i = 0
  idx = ''
  idxs = []
  for c in inds:
    i+=1
    idx+=c
    if i%3 == 0:
      idxs.append(idx)
      idx = ''
  return [name,idxs]

def setIndexCode(s):
  if len(s[1])==2:
    return 'setIndexName2('+s[0]+','+','.join(s[1])+');\n'
  if len(s[1])==4:
    return 'setIndexName4('+s[0]+','+','.join(s[1])+');\n'
  print s
  exit(0)
  return 'error'

def multCode(e,id):
  s1 = str2nmidx(e[1])
  s2 = str2nmidx(e[3][0])
  s3 = str2nmidx(e[3][1])
  str = ''
  str += setIndexCode(s1)
  str += setIndexCode(s2)
  str += setIndexCode(s3)
  str += 'm['+repr(id)+'] = Multiplication('+s1[0]+','+s2[0]+','+s3[0]+','+repr(coef)+');\n'
  return str

def summCode(e,id):
  s1 = str2nmidx(e[1])
  s2 = str2nmidx(e[3][0])
  str = ''
  str += setIndexCode(s1)
  str += setIndexCode(s2)
  str += 's['+repr(id)+'] = Summation1('+s1[0]+','+s2[0]+','+repr(coef)+');\n'
  return str

def generateCode(e,idm,ids):
  if e[1][0]=='M':
    return multCode(e[1],idm)
  if e[1][0]=='S':
    return summCode(e[1],ids)

str = ''
str2 = ''
for t in tsrmap:
  ids = tsrmap[t]
  str += 'Tensor '+t+' = Tensor'+repr(len(ids))+'('+','.join(ids)+');\n'
  str2 += 'GA_Destroy(' + t + '.handle());\n'

w = open('code/create.txt','w');
w.write(str)
w.close()

w = open('code/destroy.txt','w');
w.write(str2)
w.close()

exe = ''
expr = ''
idm=0
ids=0
curr=1
for e in exps2:
#  print e[1]
  expr += generateCode(e,idm,ids)+'\n'
  lev = e[0]
  if lev!=curr:
    curr=lev
    exe += '// barrier\n'
  if e[1][0]=='M':
    exe += 'exec(MUL,'+repr(idm)+',gtc'+repr(lev)+',task_class);\n'
    idm+=1
  if e[1][0]=='S':
    exe += 'exec(SUM,'+repr(ids)+',gtc'+repr(lev)+',task_class);\n'
    ids+=1

w = open('code/execute.txt','w');
w.write(exe)
w.close()

w = open('code/expression.txt','w');
w.write(expr)
w.close()

w = open('param.h','w')
w.write('#ifndef _PARAM_H_\n')
w.write('#define _PARAM_H_\n')
w.write('#define NIDX '+repr(len(idxnames))+'\n')
w.write('#define NOCC '+repr(nocc)+'\n')
w.write('#define NVRT '+repr(nvrt)+'\n')
w.write('typedef enum {MUL,SUM} ExprType;\n')
w.write('typedef enum {HOL,PAR} IndexType;\n')
w.write('typedef enum {')
w.write(','.join(idxnames))
w.write('} IndexName;\n')
w.write('#define NMULT '+repr(idm)+'\n')
w.write('#define NSUMM '+repr(ids)+'\n')
w.write('#endif\n')
w.close()


