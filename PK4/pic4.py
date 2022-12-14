import re
import usb.core
import usb.util
import json 

'''
<scripts>
  <commit>7acb7c9d66</commit>
  <version>000448</version>
  <script>
    <function>InitSWD</function>
    <processor>ATSAMA5D21</processor>
    <ri4command>0x00000215</ri4command>
    <scrbytes>
        <byte>0x91</byte>
    </scrbytes>
  </script>    
'''

DIC = {}
DIC['PIC'] = 'PIC24FJ256GB206'

def get_scr(f):
    SCR = []
    scr = ''
    while True:
        LL = f.readline() 
        if '</scrbytes>' in LL: 
            return SCR
        LL = LL.strip().replace('<byte>', '').replace('</byte>', '')#.replace('0x', '')
        #SCR.append(LL)
        scr += LL + ' '
        SCR.append(int(LL,0))

def get_script(f):
    global DIC
    L = f.readline()
    L += f.readline()
    L += f.readline()
    if '<processor>PIC24FJ256GB206' not in L: return    
    SCR = []
    while True:
        LL = f.readline()  
        if '</script>' in LL: 
            name = re.search('%s(.*)%s' % ('<function>', '</function>'), L).group(1)
            print('FUNCTION :', name)    
            print('SCRIPT   :', SCR)  
            DIC[name] = SCR      
            break
        SCR = get_scr(f)
        L += LL
    print()

f = open('C:/Program Files/Microchip/MPLABX/v5.50/packs/Microchip/PICkit4_TP/1.8.1120/firmware/scripts.xml', 'r')
while True:
    line = f.readline()
    if '' == line: break
    if '<script>' in line:
        get_script(f)


f.close()
print(DIC)
json_object = json.dumps(DIC, indent = 4) 
with open('PIC24FJ256GB206.json', 'w') as outfile: outfile.write(json_object)
print('[EXIT]')
exit(0)




dev = usb.core.find(idVendor=0x04D8, idProduct=0x9012)
#print( dev )
dev.set_configuration()

dev.write(0x02, b'/xE1', 100)
res = dev.read(0x81, 0x200, 100)
print( str(res[32:47], 'utf-8') )


print('------------')