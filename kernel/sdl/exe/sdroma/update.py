import os;
import random;

target = 'now.dmp';

def tryGetNewDump(fn):
    """Dump data from driver"""
    # dump data from driver.
    cwd = os.getcwd();
    os.system("rm newdump.dmp");
    os.system("rm %s" % fn);
    if (not os.path.isdir("E:\\Sora\\dot11a")):
        return False;
    os.system("E:\\Sora\\dot11a\\macconfig dumprf %s\\newdump.dmp" % cwd);
    # wait for new dump complete
    # os.system("sleep 2");
    while True:
        if (os.path.isfile("newdump.dmp") and (
            os.path.getsize("newdump.dmp") >= 16777216)):
            break;
        if (os.path.isfile("newdump.dmp")):
            print os.path.getsize("newdump.dmp");
        else:
            pass;
            #print "not file yet";
    os.system("ren newdump.dmp %s" % fn);
    if (os.path.isfile(fn)):
        return True;
    return False;

def tryGetFromStorage(fn):
    """Get a backup file from backup folder"""
    dir = 'backup';
    dmps = os.listdir(dir);
    count = len(dmps);
    index = random.randint(0, count - 1);
    os.system("copy backup\\%s %s" % (dmps[index], fn));
    return True;

def updateTarget():
    if (tryGetNewDump(target)):
        return True;
    elif (tryGetFromStorage(target)):
        return True;
    return False;

# tryGetNewDump("hello.dmp");
# print os.getcwd();
updateTarget();
