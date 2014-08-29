import sys
from subprocess import call

args = sys.argv
f = open(args[1],'r')

exps = []

for line in f:
#  print line
  if line[0]=='#': continue
  line = line.strip()
  line = line.split('+=')
  lhs = line[0]
  rhs = line[1].split('*')
  rhs = map(lambda r: r.strip(), rhs)
  n = len(rhs)
  #print 'x',n,rhs,rhs[0][-1]
  if rhs[0][-1] != ']':
    str = rhs[0]
    coef = float(str)
    rhs.pop(0)
  else:
    coef = 1.0
  lhs = lhs.strip(' ')
  #print 'rhs', rhs
  if len(rhs)==1:
    e = ['S',coef,lhs,rhs]
  elif len(rhs)==2:
    e = ['M',coef,lhs,rhs]
  else:
    print 'error 1'
    exit(0)
  exps.append(e)

tensorNames = set()
indexNames = set()
tensorDims = {}

def parseTensor(str,tensorNames,indexNames,tensorDims):
  x = str.split('[')
  name = x[0]
  indexStr = x[1][:-1]
  indices = indexStr.split(',')
  ranges = []
  for i in indices:
    if i[0]=='p':
      ranges.append('PAR')
    elif i[0]=='h':
      ranges.append('HOL')
    else:
      print i[0],'error 2'
      exit(0)
    indexNames.add(i)
  tensorNames.add(name)
  tensorDims[name] = ranges
  return

for e in exps:
  #print e
  #print e[2]
  #print e[3]
  parseTensor(e[2],tensorNames,indexNames,tensorDims)
  for r in e[3]:
    parseTensor(r,tensorNames,indexNames,tensorDims)

indexNames = list(indexNames)
indexNames.sort()
tensorNames = list(tensorNames)
tensorNames.sort()

nocc = 0
nvrt = 1

for i in indexNames:
  if i[0]=='h':
    nocc += 1
  elif i[0]=='p':
    nvrt += 1

level = []

for e in exps:
  level.append(0)

check = ''
for e in exps:
  for f in e[3]:
    for g in exps:
      #print f,g[1]
      n1 = f.split('[')
      n2 = g[2].split('[')
      if n1[0]==n2[0]: # find dependency, assign level
        level[exps.index(e)]=max(level[exps.index(g)]+1,level[exps.index(e)])

layer = []
for i in range(max(level) + 1):
  l = []
  for e in exps:
    elv = level[exps.index(e)]
    if elv == i:
      l.append(e)
  layer.append(l)

#print layer[0]
#print layer[1]
#print indexNames
#print tensorNames
#print tensorDims
#print layer

def str2NameIndex(str):
  x = str.split('[')
  name = x[0]
  indexStr = x[1][:-1]
  indices = indexStr.split(',')
  return [name,indices]

def setIndexCode(s):
  if len(s[1])==2:
    return 'setIndexName2(T['+s[0]+'], '+', '.join(s[1])+');\n'
  if len(s[1])==4:
    return 'setIndexName4(T['+s[0]+'], '+', '.join(s[1])+');\n'
#  print s
  exit(0)
  return 'error 3'

def genMUL(e,id):
  coef = e[1]
  c = str2NameIndex(e[2])
  a = str2NameIndex(e[3][0])
  b = str2NameIndex(e[3][1])
  str = ''
  str += setIndexCode(a)
  str += setIndexCode(b)
  str += setIndexCode(c)
  str += 'm['+repr(id)+'] = Multiplication(T['+c[0]+'], T['+a[0]+'], T['+b[0]+'], '+repr(coef)+');\n'
  return str

def genSUM(e,id):
  coef = e[1]
  s1 = str2NameIndex(e[2])
  s2 = str2NameIndex(e[3][0])
  str = ''
  str += setIndexCode(s1)
  str += setIndexCode(s2)
  str += 's['+repr(id)+'] = Summation(T['+s1[0]+'], T['+s2[0]+'], '+repr(coef)+');\n'
  return str

def genExprCode(e,eid):
  if e[0]=='M':
    return genMUL(e,eid)
  if e[0]=='S':
    return genSUM(e,eid)

tsrCode = ''
tsrCode += 'enum TensorName{\n'
tsrCode += ','.join(tensorNames)
tsrCode += '};\n'
tsrCode += 'numTensors = ' + repr(len(tensorDims)) +';\n'
#tsrCode += 'vector<Tensor> T(numTensors);\n'
tsrCode += 'T.resize(numTensors);\n'
for t in tensorDims:
  ids = tensorDims[t]
  tsrCode += 'T['+t+'] = Tensor'+repr(len(ids))+'('+', '.join(ids)+', '+t+');\n'

exprCode = ''
idm = 0
ids = 0
for e in exps:
  if e[0]=='M':
    e.append(idm)
    idm+=1
  if e[0]=='S':
    e.append(ids)
    ids+=1
  exprCode += genExprCode(e,e[-1])

execCode = ''
lv = 0

for l in layer:
  lv += 1
  execCode += '\nBarrier();\n'
  execCode += 'if (!me) { cout << \"layer '+repr(lv)+'\\n\"; cout.flush(); }\n'
  execCode += 'TaskPool tp'+repr(lv)+' = gtc_create(AUTO_BODY_SIZE, NUM_TASKS, NULL, MPI_COMM_WORLD);\n'
  for e in l:
    if e[0]=='M':
      execCode += 'exec(MUL,'+repr(e[-1])+',tp'+repr(lv)+');\n'
    if e[0]=='S':
      execCode += 'exec(SUM,'+repr(e[-1])+',tp'+repr(lv)+');\n'
  execCode += '#if TIMER\n'
  execCode += 'MyTimer::start(TOT);\n'
  execCode += '#endif\n'
  execCode += 'gtc_process(tp'+repr(lv)+');\n'
  execCode += '#if TIMER\n'
  execCode += 'MyTimer::stop(TOT);\n'
  execCode += '#endif\n\n'


param = ''
#param += '#ifndef _PARAM_H_\n'
#param += '#define _PARAM_H_\n'
param += '#define NIDX '+repr(len(indexNames))+'\n'
param += '#define NOCC '+repr(nocc)+'\n'
param += '#define NVRT '+repr(nvrt)+'\n'
#param += 'typedef enum {MUL,SUM} ExprType;\n'
#param += 'typedef enum {HOL,PAR} IndexType;\n'
param += 'enum IndexName{'
param += ','.join(indexNames)
param += '};\n'
param += '#define NMULT '+repr(idm)+'\n'
param += '#define NSUMM '+repr(ids)+'\n'
#param += '#endif\n'

w = open('param.txt','w')
w.write(param)
w.close()

w = open('code.txt','w')
w.write(tsrCode)
w.write(exprCode)
w.write(execCode)
w.close()






