

var segGroups = {};
var childToType = [];


firstLower = function(str)
{
    return str.charAt(0).toLowerCase() + str.slice(1);
}

title = function(str)
{
    return str.charAt(0).toUpperCase() + str.slice(1).toLowerCase();
}


addFlagStruct = function(obj, flag)
{
    if (!obj.flagsStruct)
    {
        obj.flagsStruct = [flag];
    }
    else
    {
        obj.flagsStruct.push(flag);
    }
}

addFlagField = function(obj, flag)
{
    if (!obj.flagsField)
    {
        obj.flagsField = [flag];
    }
    else
    {
        obj.flagsField.push(flag);
    }
}


isTypeAvailabe = function(structsOrdered, type)
{
    for (var i = 0; i < structsOrdered.length; ++i)
    {
        if (structsOrdered[i].type == type)
        {
            return true;
        }
    }
    return false;
}

isDependencyOk = function (structsOrdered, stru)
{
    for (var i = 0; i < stru.fields.length; ++i)
    {
        var field = stru.fields[i];
        if (field.tid == 'struct' || field.tid == 'struct[]')
        {
            if (!isTypeAvailabe(structsOrdered, field.type))
            {
                return false;
            }
        }
    }
    return true;
}


sortDependency = function (structs)
{
    var structsOrdered = [];
    
    while (structs.length > 0)
    {
        for (var i = 0; i < structs.length; ++i)
        {
            var stru = structs[i];
            if (isDependencyOk(structsOrdered, stru))
            {
                structsOrdered.push(stru);
                structs.splice(i, 1);
                --i;
            }
        }
    }
    return structsOrdered;
}




module.exports = {

    makeFieldName: function (desc)
    {
        var name = desc;
        name = name.replaceAll('/', '_');
        name = name.replaceAll('-', '_');
        name = name.replaceAll('"', '');
        name = name.replaceAll(',', '_');
        name = name.replaceAll(':', ' ');
        name = name.replaceAll('.', ' ');
        name = name.replaceAll('&', ' ');
        name = name.replaceAll('+', ' ');
        name = name.replaceAll('*', ' ');
        name = name.replaceAll('#', ' ');
        name = name.replaceAll("'", ' ');
        names = name.split(' ')
        for (var i in names)
        {
            names[i] = title(names[i]);
        }
        name = names.join('');
        name = firstLower(name);
        name = name.split('(')[0];
        if (name == 'event')
        {
            name = 'ev';
        }
        else if (name == 'operator')
        {
            name = 'op';
        }
        return name;
    },
    
    makeFieldNames: function (hl7dictionary)
    {
        for (var fieldKey in hl7dictionary.fields)
        {
            var field = hl7dictionary.fields[fieldKey];
            var namesCollection = {}
            var namesNotUnique = {}
            for (var subfieldKey in field.subfields)
            {
                var subfield = field.subfields[subfieldKey];
                subfield.nameNotUnique = this.makeFieldName(subfield.desc);
                if (namesCollection[subfield.nameNotUnique])
                {
                    namesNotUnique[subfield.nameNotUnique] = 1;
                }
                namesCollection[subfield.nameNotUnique] = true;
            }
            for (var subfieldKey in field.subfields)
            {
                var subfield = field.subfields[subfieldKey];
                subfield.name = subfield.nameNotUnique;
                if (namesNotUnique[subfield.name])
                {
                    var id = namesNotUnique[subfield.name];
                    ++namesNotUnique[subfield.name];
                    subfield.name += '_' + id;
                }
            }            
        }        

        for (var segmentKey in hl7dictionary.segments)
        {
            var segment = hl7dictionary.segments[segmentKey];
            var namesCollection = {}
            var namesNotUnique = {}
            for (var fieldKey in segment.fields)
            {
                var field = segment.fields[fieldKey];
                field.nameNotUnique = this.makeFieldName(field.desc);
                if (namesCollection[field.nameNotUnique])   
                {
                    namesNotUnique[field.nameNotUnique] = 1;
                }
                namesCollection[field.nameNotUnique] = true;
            }
            var id = 1;
            for (var fieldKey in segment.fields)
            {
                var field = segment.fields[fieldKey];
                field.name = field.nameNotUnique;
                if (namesNotUnique[field.name])
                {
                    var id = namesNotUnique[field.name];
                    ++namesNotUnique[field.name];
                    field.name += '_' + id;
                }
            }            
        }        

        for (var keyMessage in hl7dictionary.messages)
        {
            var message = hl7dictionary.messages[keyMessage];
            var children = message.segments.segments;
            this.processChildrenNames(children);
        }
    },
    
    isStruct: function(fields, typename)
    {
        var type = fields[typename];
        if (type != null)
        {
            if (Object.keys(type.subfields).length == 0)
            {
                return false;
            }
            return true;
        }
        return false;
    },
    
    buildSegGroups: function (hl7dictionary)
    {
        for (var keyMessage in hl7dictionary.messages)
        {
            var message = hl7dictionary.messages[keyMessage];
            var children = message.segments.segments;
            this.processChildren(children);
        }

        for (var i = 0; i < childToType.length; i++)
        {
            var entry = childToType[i];
            entry.type = entry.type.replaceAll(',', '_');
            entry.type = entry.type.replaceAll('/', '_');
            entry.child.type = entry.type;
            entry.child.name = entry.child.name.replaceAll(',', '_');
            entry.child.name = entry.child.name.replaceAll('/', '_');
        }
        

        for (var keyMessage in hl7dictionary.messages)
        {
            var message = hl7dictionary.messages[keyMessage];
            for (var keySegment in message.segments.segments)
            {
                var children = message.segments.segments;
                this.processChildrenType(children);
            }
        }

        hl7dictionary.segGroups = segGroups;
        
        

    },
    
    processChildren: function (children)
    {
        for (var keyChild in children)
        {
            var child = children[keyChild];
            if (child.children || child.compounds)
            {
                if (!(child.name in segGroups))
                {
                    var type = child.name + '_' + 1;
                    segGroups[child.name] = [{type:type, child:child}];
                    childToType.push({type:type, child:child});
                }
                else
                {
                    var typeExist = null;
                    var found = false;
                    for (var i = 0; i < segGroups[child.name].length; ++i)
                    {
                        if (JSON.stringify(segGroups[child.name][i].child) == JSON.stringify(child))
                        {
                            typeExist = segGroups[child.name][i].type;
                            found = true;
                            break;
                        }
                    }
                    if (!found)
                    {
                        var type = child.name + '_' + (segGroups[child.name].length + 1);
                        segGroups[child.name].push({type:type, child:child});
                        childToType.push({type:type, child:child});
                    }
                    else
                    {
                        childToType.push({type:typeExist, child:child});
                    }
                }

                if (child.children)
                {
                    this.processChildren(child.children);
                }
                else if (child.compounds)
                {
                    this.processChildren(child.compounds);
                }
                
            }
        }        
    },
    
    putFlags: function (hl7dictionary)
    {
        for (var keyMessage in hl7dictionary.messages)
        {
            var message = hl7dictionary.messages[keyMessage];
            var children = message.segments.segments;
            this.processChildrenFlags(children, false);
        }
    },
    
    processChildrenFlags: function (children, choice)
    {
        for (var keyChild in children)
        {
            var child = children[keyChild];
            
            if (child.children)
            {
                var c = (child.compounds) ? true : false;
                if (c)
                {
                    addFlagStruct(child, 'METASTRUCTFLAG_CHOICE');
                }
                this.processChildrenFlags(child.children, c);
            }

            if (choice || (child.min == 0 && child.max == 1))
            {
                addFlagField(child, 'METAFLAG_NULLABLE');
            }
            if (child.min == 1 && child.max != 1)
            {
                addFlagField(child, 'METAFLAG_ONE_REQUIRED');
            }
        }
    },


    processChildrenType: function (children)
    {
        for (var keyChild in children)
        {
            var child = children[keyChild];            
            
            if (child.compounds)
            {
                child.children = child.compounds;
            }

            if (child.children)
            {
                this.processChildrenType(child.children);
            }
            else
            {
                child.name = child.name.replaceAll(',', '_');
                child.type = child.name;
            }
        }
    },

    processChildrenNames: function (children)
    {
        var namesCollection = {}
        var namesNotUnique = {}
        for (var keyChild in children)
        {
            var child = children[keyChild];
            
            child.name = child.name.toLowerCase();
            
            nameNotUnique = child.name;
            if (namesCollection[nameNotUnique])
            {
                namesNotUnique[nameNotUnique] = 1;
            }
            namesCollection[nameNotUnique] = true;

            if (child.children)
            {
                this.processChildrenNames(child.children);
            }
        }

        for (var keyChild in children)
        {
            var child = children[keyChild];
            if (namesNotUnique[child.name])
            {
                var id = namesNotUnique[child.name];
                ++namesNotUnique[child.name];
                child.name += '_' + id;
            }
        }            
        
    },
    
    
    generateData: function (hl7dictionary)
    {        
        var data = {namespace: 'hl7',
                    enums: [],
                    structs: [] };

        var structsHl7Fields = [];
        for (var key in hl7dictionary.fields) { 
            var field = hl7dictionary.fields[key]
            var stru = {type: key, desc: field.desc, fields:[]};
            for (var i = 0; i < field.subfields.length; ++i) { 
                var subfield = field.subfields[i];
                if (this.isStruct(hl7dictionary.fields, subfield.datatype))
                {
                    stru.fields.push({tid:'struct', type: subfield.datatype, name: subfield.name});
                } 
                else 
                {
                    stru.fields.push({tid:'string', type: '', name: subfield.name});
                }
            }
            data.structs.push(stru);
        }
        
        for (var key in hl7dictionary.segments) { 
            var segment = hl7dictionary.segments[key]
            var stru = {type: key, desc: segment.desc, flags:["METASTRUCTFLAG_HL7_SEGMENT"], fields:[]};
            for (var i = 0; i < segment.fields.length; ++i) { 
                var field = segment.fields[i]
                var strArray = '';
                if (field.rep != 1)
                {
                    strArray = '[]';
                }
                if (this.isStruct(hl7dictionary.fields, field.datatype)) 
                {
                    stru.fields.push({tid:'struct'+strArray, type: field.datatype, name: field.name});
                } 
                else
                {
                    stru.fields.push({tid:'string'+strArray, type: '', name: field.name});
                }
            }
            data.structs.push(stru);
        }
        
        for (var key in hl7dictionary.segGroups) { 
            var segGroup = hl7dictionary.segGroups[key]
            for (var i = 0; i < segGroup.length; ++i) { 
                var g = segGroup[i].child;
                var stru = {type: g.type, desc: g.desc.replaceAll('"', '')};
                if (g.flagsStruct)
                {
                    stru.flags = g.flagsStruct;
                }
                stru.fields = [];
                for (var n = 0; n < g.children.length; ++n) {
                    var child = g.children[n];
                    var strArray = '';
                    if (child.max != 1 && !g.compounds)
                    {
                        strArray = '[]';
                    }
                    var field = {tid: 'struct'+strArray, type: child.type, name: child.name};
                    if (child.flagsField)
                    {
                        field.flags = child.flagsField;
                    }
                    stru.fields.push(field);
                }
                data.structs.push(stru);
            }
        }

        var keysMessages = Object.keys(hl7dictionary.messages);
        for (var n = 0; n < keysMessages.length; ++n) { 
            var key = keysMessages[n];
            var message = hl7dictionary.messages[key];
            var stru = {type: key, desc: message.desc.replaceAll('"', '')};
            if (message.flagsStruct)
            {
                stru.flags = message.flagsStruct;
            }
            stru.fields = [];
            for (var i = 0; i < message.segments.segments.length; ++i) { 
                var segment = message.segments.segments[i]
                var strArray = '';
                if (segment.max != 1)
                {
                    strArray = '[]';
                }
                var field = {tid: 'struct'+strArray, type: segment.type, name: segment.name};
                if (segment.flagsField)
                {
                    field.flags = segment.flagsField;
                }
                stru.fields.push(field);
            }
            data.structs.push(stru);
        }

        data.structs = sortDependency(data.structs);

        return data;
    }

}

