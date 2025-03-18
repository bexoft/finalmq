var g_fmqSession = new FmqSession();
var g_fmqLoggingServer = null;


function priorityToColor(priority)
{
    if (priority == 0) return "rgb(200,200,200)";
    if (priority == 1) return "rgb(120,120,200)";
    if (priority == 2) return "rgb(120,120,200)";
    if (priority == 3) return "rgb(  0,  0,  0)";
    if (priority == 4) return "rgb(  0,  0,  0)";
    if (priority == 5) return "rgb(255,140,  0)";
    if (priority == 6) return "rgb(255,  0,  0)";
    if (priority == 7) return "rgb(200,  0,  0)";
    if (priority == 8) return "rgb(180,  0,  0)";
    return 'black';
}

function priorityTextToNumber(priorityText)
{
    if (priorityText == 'LOG_NONE') return 0;
    if (priorityText == 'LOG_TRACE') return 1;
    if (priorityText == 'LOG_DEBUG') return 2;
    if (priorityText == 'LOG_INFO') return 3;
    if (priorityText == 'LOG_NOTICE') return 4;
    if (priorityText == 'LOG_WARNING') return 5;
    if (priorityText == 'LOG_ERROR') return 6;
    if (priorityText == 'LOG_CRITICAL') return 7;
    if (priorityText == 'LOG_FATAL') return 8;
    return 'black';
}




function methodCallTableEntry(methodname, inparams)
{
    var d = new Date(inparams.timestamp);
    var tableEntry = '';
    tableEntry += '<tr style="color:' + priorityToColor(inparams.priority) + '">';
    tableEntry += '<td style="width:200">' + d.toISOString().replace('-','').replace('-','') + '</td>';
    tableEntry += '<td>' + inparams.component + '</td>';
    tableEntry += '<td>' + inparams.context.module + '</td>';
    tableEntry += '<td>' + inparams.context.level + '</td>';
    tableEntry += '<td>' + inparams.text + '</td>';
    tableEntry += '<td>' + inparams.context.filename.replace(/^.*[\\\/]/, '') + '</td>';
    tableEntry += '<td>' + inparams.context.line + '</td>';
    tableEntry += '</tr>';
    return tableEntry;
}

class LoggingListener extends FmqEntity
{
    constructor(session, name) 
    {
        super(session, name);
        this.strTable = '';
        this.entries = [];
        this.filterComponent1 = '';
        this.filterComponent2 = '';
        this.filterPriority = 0;
        this.filterText1 = '';
        this.filterText2 = '';
        this.timer = null;
        
        this.start();
    }

    sessionDisconnected()
    {
    }

    sessionConnected()
    {
    }

    serverDisconnected()
    {
    }

    serverConnected()
    {
    }

    clear()
    {
        this.strTable = '';
        var loggingframe = document.getElementById('loggingframe');
        loggingframe.contentDocument.getElementById("logdata").innerHTML = '';
        this.entries = [];
    }
    
    stop()
    {
        if (this.timer != null)
        {
            clearTimeout(this.timer);
            this.timer = null;
        }
    }
    
    start()
    {
        if (this.timer == null)
        {
            this.timer = setInterval(this.updateTable, 1000, this);
        }
    }
    
    logFilter = function(entry)
    {
        if (!((this.filterComponent1 == '' && this.filterComponent2 == '') ||
              ((entry.component == this.filterComponent1) || (entry.component == this.filterComponent2))))
        {
            return false;
        }
        if (!((this.filterText1 == '') ||
              (entry.text.match(this.filterText1))))
        {
            return false;
        }
        if (!((this.filterText2 == '') ||
              (entry.text.match(this.filterText2))))
        {
            return false;
        }
        
        
        if (!((this.filterPriority == 0) ||
              (priorityTextToNumber(entry.context.level) >= this.filterPriority)))
        {
            return false;
        }
        return true;
    }

    updateTable = function(obj)
    {
        if (obj.strTable != '')
        {
            var loggingframe = document.getElementById('loggingframe');
            var scrolledAtEnd = (loggingframe.contentWindow.pageYOffset >= loggingframe.contentDocument.body.scrollHeight - loggingframe.contentWindow.innerWidth + 10);
            loggingframe.contentDocument.getElementById("logdata").innerHTML += obj.strTable;
            obj.strTable = '';
            if (scrolledAtEnd)
            {
                loggingframe.contentWindow.scrollTo(0, loggingframe.contentDocument.body.scrollHeight);
            }
        }
    }
    
    getPriorityValue = function(id)
    {
        var el = document.getElementById(id);
        if (el != null)
        {
            if (el.checked)
            {
                return el.value;
            }
        }
        return 0;
    }
    
    getPriority()
    {
        var priority = this.getPriorityValue("ID_FILTERTRACE");
        if (priority == 0)
        {
            priority = this.getPriorityValue("ID_FILTERDEBUG");
        }
        if (priority == 0)
        {
            priority = this.getPriorityValue("ID_FILTERINFO");
        }
        if (priority == 0)
        {
            priority = this.getPriorityValue("ID_FILTERNOTICE");
        }
        if (priority == 0)
        {
            priority = this.getPriorityValue("ID_FILTERWARN");
        }
        if (priority == 0)
        {
            priority = this.getPriorityValue("ID_FILTERERROR");
        }
        if (priority == 0)
        {
            priority = this.getPriorityValue("ID_FILTERCRIT");
        }
        if (priority == 0)
        {
            priority = this.getPriorityValue("ID_FILTERFATAL");
        }
        return priority;
    }
    
    applyFilter()
    {
        this.filterPriority = this.getPriority();
        this.filterComponent1 = document.getElementById("ID_FILTERCOMPONENT1").value;
        this.filterComponent2 = document.getElementById("ID_FILTERCOMPONENT2").value;
        this.filterText1 = document.getElementById("ID_FILTERTEXT1").value;
        this.filterText2 = document.getElementById("ID_FILTERTEXT2").value;
        this.strTable = '';
        var loggingframe = document.getElementById('loggingframe');
        loggingframe.contentDocument.getElementById("logdata").innerHTML = '';
        for (var i in this.entries)
        {
            var entry = this.entries[i];
            if (this.logFilter(entry))
            {
                this.strTable += methodCallTableEntry('log', entry);
            }
        }
        this.updateTable(this);
    }

    log(correlationId, inparams)
    {
        this.entries.push(inparams);
        if (this.logFilter(inparams))
        {
            this.strTable += methodCallTableEntry('log', inparams);
        }
    }

}


function loadPage() {
    g_fmqSession.createSession(function() {
        g_fmqLoggingServer = g_fmqSession.createEntity('LoggingServer', LoggingListener);
    });
}

function unloadPage() {
    g_fmqSession.removeSession();
}
