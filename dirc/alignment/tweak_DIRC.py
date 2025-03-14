import xml.etree.ElementTree
import sys,configparser

def rot_TSM(root,optical_box,TSM_number,perturb_values,verbose=False):
    for volume in root.findall("./composition"):
        if volume.attrib['name']== optical_box:
            for subvolume in volume.findall('./posXYZ'):
                if subvolume.attrib['volume']=='TSM%s'%TSM_number:
                    if verbose:
                        print("Nominal:\n",subvolume.tag, subvolume.attrib)
                    rot_values = list(map(float ,subvolume.attrib['rot'].split()))
                    rot_str_modified = '%s %s %s'%(rot_values[0]+perturb_values[0],
                                                   rot_values[1]+perturb_values[1],
                                                   rot_values[2]+perturb_values[2])
                    subvolume.attrib['rot'] = rot_str_modified
                    if verbose:
                        print("After tweaks:\n",subvolume.tag, subvolume.attrib)

def rot_MRA(root,optical_box,perturb_values,verbose=False):
    for volume in root.findall("./composition"):
        if volume.attrib['name']== optical_box:
            for subvolume in volume.findall('./posXYZ'):
                if subvolume.attrib['volume']=='MRA%s'%optical_box[-1]:
                    if verbose:
                        print("Nominal:\n",subvolume.tag, subvolume.attrib)
                    rot_values = list(map(float ,subvolume.attrib['rot'].split()))
                    rot_str_modified = '%s %s %s'%(rot_values[0]+perturb_values[0],
                                                   rot_values[1]+perturb_values[1],
                                                   rot_values[2]+perturb_values[2])
                    subvolume.attrib['rot'] = rot_str_modified
                    if verbose:
                        print("After tweaks:\n",subvolume.tag, subvolume.attrib)


def offset_MRA(root,optical_box,perturb_values,verbose=False):
    for volume in root.findall("./composition"):
        if volume.attrib['name']== optical_box:
            for subvolume in volume.findall('./posXYZ'):
                if subvolume.attrib['volume']=='MRA%s'%optical_box[-1]:
                    if verbose:
                        print("Nominal:\n",subvolume.tag, subvolume.attrib)
                    offset_values = list(map(float ,subvolume.attrib['X_Y_Z'].split()))
                    offset_str_modified = '%s %s %s'%(offset_values[0]+perturb_values[0],
                                                      offset_values[1]+perturb_values[1],
                                                      offset_values[2]+perturb_values[2])
                    subvolume.attrib['X_Y_Z'] = offset_str_modified
                    if verbose:
                        print("After tweaks:\n",subvolume.tag, subvolume.attrib)

def main(config):
    if config.get('FILES','NOMINAL')== None:
        print("File NOMINAL reading failed! Please check config file")
        exit()
    tree = xml.etree.ElementTree.parse(config.get('FILES','NOMINAL'))
    root = tree.getroot()
    for section in config.sections():
        if section == 'OBCS':
            #for TSM_number in range(1,4):
            #    MRAS_TSM_rot = list(map(float,config.get('OBCS','TSM%s_rot'%TSM_number).split(',')))
            #    rot_TSM(root,section,TSM_number,MRAS_TSM_rot)
            MRAS_rot = list(map(float,config.get('OBCS','MRAS_rot').split(',')))
            rot_MRA(root,section,MRAS_rot)
            MRAS_offset = list(map(float,config.get('OBCS','MRAS_offset').split(',')))
            offset_MRA(root,section,MRAS_offset)
        if section == 'OBCN':
            #for TSM_number in range(1,4):
            #    MRAN_TSM_rot = list(map(float,config.get('OBCN','TSM%s_rot'%TSM_number).split(',')))
            #    rot_TSM(root,section,TSM_number,MRAN_TSM_rot)
            MRAN_rot = list(map(float,config.get('OBCN','MRAN_rot').split(',')))
            rot_MRA(root,section,MRAN_rot)
            MRAN_offset = list(map(float,config.get('OBCN','MRAN_offset').split(',')))
            offset_MRA(root,section,MRAN_offset)
    tree.write(config.get('FILES','OUTPUT'))


if __name__ == '__main__':
    config = configparser.RawConfigParser(allow_no_value=True)
    if len(sys.argv)==1:
        print("Please provide a config file. Run as:\n $python tweak_DIRC.py config.cfg")
        exit()	
    config.read(sys.argv[1])
    main(config)

