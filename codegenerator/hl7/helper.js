

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
            entry.child.type = entry.type;
            entry.child.name = entry.child.name.replaceAll(',', '_');
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

}

