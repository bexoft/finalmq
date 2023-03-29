var fs = require('fs')
var helper = require(__dirname + '/helper')
var argv = require('minimist')(process.argv.slice(2));

var fileOptions = argv['options']
var fileData = argv['input']
var fileTables = argv['tables']
var pathOutput = argv['outpath']

var hl7dictionary = require(fileData);

var hl7Tables = null;
if (fileTables)
{
    hl7Tables = require(fileTables);
}

var fileOutput = fileData + '.fmq'

if (!fileOptions)
{
    fileOptions = './hl7options.json';
}

var options = null;

try
{
    options = require(fileOptions);
}
catch (err)
{
}

if (pathOutput)
{
    var splitFileOutput   = fileOutput.split('/')

    fileOutput   = pathOutput + '/' + splitFileOutput[splitFileOutput.length - 1]
}

try {
    fs.mkdirSync(pathOutput, { recursive: true })
}
catch (err) {
}

helper.buildSegGroups(hl7dictionary);
helper.makeFieldNames(hl7dictionary);
helper.putFlags(hl7dictionary);

if (hl7Tables)
{
    helper.processTables(hl7Tables);
}

var data = helper.generateData(hl7dictionary, hl7Tables, options);
var str = JSON.stringify(data, null, 4);


str = str.replaceAll('\r\n                }', ' }');
str = str.replaceAll('\r                }', ' }');
str = str.replaceAll('\n                }', ' }');

str = str.replaceAll('\r\n            "', ' "');
str = str.replaceAll('\r            "', ' "');
str = str.replaceAll('\n            "', ' "');

str = str.replaceAll('\r\n                "', ' "');
str = str.replaceAll('\r                "', ' "');
str = str.replaceAll('\n                "', ' "');

str = str.replaceAll('\r\n                    "', ' "');
str = str.replaceAll('\r                    "', ' "');
str = str.replaceAll('\n                    "', ' "');

str = str.replaceAll('\r\n                        "', ' "');
str = str.replaceAll('\r                        "', ' "');
str = str.replaceAll('\n                        "', ' "');

str = str.replaceAll('\r\n            ],', ' ],');
str = str.replaceAll('\r            ],', ' ],');
str = str.replaceAll('\n            ],', ' ],');

str = str.replaceAll('\r\n                    ]', ' ]');
str = str.replaceAll('\r                    ]', ' ]');
str = str.replaceAll('\n                    ]', ' ]');


fs.writeFileSync(fileOutput, str, 'utf8');


