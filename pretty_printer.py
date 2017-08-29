import gdb.printing
POLLIN=1
POLLOUT=4
# I want to use it like this
# fields_ = ['fd_', 'loop_']
# specialCases = [('revent_',formatter)]
def formatObject( gdbVal, fields, specialCases):
    def helper( gdbVal,field, formatter=None):
        if formatter is None:
            return '{}={} '.format(field,gdbVal.val[field])
        else :
            return '{}={} '.format(field,formatter(gdbVal.val[field]))
    formattedStr = ''
    for field in fields:
        formattedStr+=helper(gdbVal,field)
    for pair in specialCases:
        formattedStr+=helper(gdbVal,pair[0],pair[1])
    return '{'+formattedStr+'}'



def formatEvents( gdbVal):
    events = int(gdbVal) #contract
    ret = ''
    if events & POLLIN:
        ret+='|POLLIN'
    if events & POLLOUT:
        ret+='|POLLOUT'
    if (ret==''):
        ret='NoEvent'
    return '{}({})'.format(str(events),ret)
class ChannelPrinter:
    def __init__ (self,val):
        self.val = val;

    #@staticmethod

    def to_string(self):
        return formatObject(self, ['fd_','loop_'],
                [('events_', formatEvents),
                 ('revents_',formatEvents)]);

def register_printer():
    pp = gdb.printing.RegexpCollectionPrettyPrinter("libwz")
    pp.add_printer('Channel', 'Channel', ChannelPrinter )
    gdb.printing.register_pretty_printer(
        gdb.current_objfile(),pp,True) #replace the old one
    print ("printer loaded")

register_printer();
