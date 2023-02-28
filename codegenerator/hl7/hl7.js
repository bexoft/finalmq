var fs = require('fs')
var helper = require(__dirname + '/helper')
var argv = require('minimist')(process.argv.slice(2));

var fileOptions = argv['options']
var fileData = argv['def']
var pathOutput = argv['outpath']

var hl7dictionary = require(fileData)

var fileOutput = fileData + '.fmq'

if (!fileOptions)
{
    fileOptions = './options.json';
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

helper.buildSegGroups(hl7dictionary);
helper.makeFieldNames(hl7dictionary);
helper.putFlags(hl7dictionary);

var data = helper.generateData(hl7dictionary, options);
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


