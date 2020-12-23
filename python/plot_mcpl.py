#exec(open('python/remove_low_weight_mcpl.py').read())
import mcpl
import sys
import pandas as pd
import matplotlib.pyplot as plt
import matplotlib
matplotlib.use('TkAgg')

from pandas.plotting import scatter_matrix


filename = '/tmp/ILL_2.6e6/H512_Vout.mcpl.gz' #sys.argv[1]
myfile = mcpl.MCPLFile(filename)

print( 'Number of particles in file: %i' % myfile.nparticles )
print( 'File created by: "%s"' % myfile.sourcename )
if myfile.opt_polarisation:
    print( 'File contains polarisation information' )
for c in myfile.comments:
    print( 'File has comment: "%s"' % c )


#df = pd.DataFrame(columns = ['x','y','z', 'ux','uy','uz','weight', 'time'])

dfs = []
for pblock in myfile.particle_blocks:
    data = { 'x' : pblock.x,
             'y' : pblock.y,
             'z' : pblock.z,
             'ux': pblock.ux,
             'uy': pblock.uy,
             'uz': pblock.uz,
             'weight': pblock.weight,
             'time': pblock.time,
             'ekin':pblock.ekin             
             }

    dfs.append(
        pd.DataFrame( data,
                      index=range(pblock.file_offset, pblock.file_offset+len(pblock))
                      )
        )

df = pd.concat(dfs)

print(df.head())
#scatter_matrix(df, figsize=(20,20), range_padding=0.10, diagonal='hist', alpha=0.3)
plt.clf()
dir='/tmp/'
extension='png'

for c in df.columns:
    xplot = df[c].plot(kind='hist')
    xplot.set_xlabel=c
    plt.savefig(dir+'/'+c+'.'+extension, dpi=200)
    plt.clf()
#plt.figure()

#yplot = df['y'].plot(kind='hist')
#yplot.xaxis.set_label_text='y'
#plt.savefig('/tmp/fig.png', dpi=200)
#df.plot(kind='scatter', subplots=True, layout=(3,3), logy=True)
#plt.show()
#filtered = [ (p.weight, p.x, p.y) for p in myfile.particles if p.weight==0 ] 
#        print( p.weight, p.x, p.y, p.z, p.ekin,  p.time )
