module.exports = {

    makeFieldName: function (desc)
    {
        var name = desc;
        name = name.replaceAll(' ', '');
        name = name.replaceAll('/', '_');
        name = name.replaceAll('-', '_');
        name = name.replaceAll('"', '');
        name = name.replaceAll(',', '_');
        name = name.replaceAll(':', '');
        name = name.replaceAll('.', '');
        name = name.replaceAll('&', '');
        name = name.replaceAll('+', '');
        name = name.replaceAll('*', '');
        name = name.replaceAll('#', '');
        name = name.replaceAll("'", '');
        name = name.charAt(0).toLowerCase() + name.slice(1);
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
                    namesNotUnique[subfield.nameNotUnique] = true;
                }
                namesCollection[subfield.nameNotUnique] = true;
            }
            var id = 1;
            for (var subfieldKey in field.subfields)
            {
                var subfield = field.subfields[subfieldKey];
                subfield.name = subfield.nameNotUnique;
                if (namesNotUnique[subfield.name])
                {
                    subfield.name += '_' + id;
                    ++id;
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
                    namesNotUnique[field.nameNotUnique] = true;;
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
                    field.name += '_' + id;
                    ++id;
                }
            }            
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

}
