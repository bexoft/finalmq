//MIT License

//Copyright (c) 2020 bexoft GmbH (mail@bexoft.de)

//Permission is hereby granted, free of charge, to any person obtaining a copy
//of this software and associated documentation files (the "Software"), to deal
//in the Software without restriction, including without limitation the rights
//to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
//copies of the Software, and to permit persons to whom the Software is
//furnished to do so, subject to the following conditions:

//The above copyright notice and this permission notice shall be included in all
//copies or substantial portions of the Software.

//THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
//SOFTWARE.


namespace finalmq
{

    public enum MetaTypeId
    {
        TYPE_NONE = 0,

        TYPE_BOOL = 1,
        TYPE_INT8 = 2,
        TYPE_UINT8 = 3,
        TYPE_INT16 = 4,
        TYPE_UINT16 = 5,
        TYPE_INT32 = 6,
        TYPE_UINT32 = 7,
        TYPE_INT64 = 8,
        TYPE_UINT64 = 9,
        TYPE_FLOAT = 10,
        TYPE_DOUBLE = 11,
        TYPE_STRING = 12,
        TYPE_BYTES = 13,
        TYPE_STRUCT = 14,
        TYPE_ENUM = 15,
        TYPE_VARIANT = 16,

        OFFSET_ARRAY_FLAG = 1024,

        TYPE_ARRAY_BOOL = OFFSET_ARRAY_FLAG + TYPE_BOOL,
        TYPE_ARRAY_INT8 = OFFSET_ARRAY_FLAG + TYPE_INT8,
        TYPE_ARRAY_UINT8 = OFFSET_ARRAY_FLAG + TYPE_UINT8,
        TYPE_ARRAY_INT16 = OFFSET_ARRAY_FLAG + TYPE_INT16,
        TYPE_ARRAY_UINT16 = OFFSET_ARRAY_FLAG + TYPE_UINT16,
        TYPE_ARRAY_INT32 = OFFSET_ARRAY_FLAG + TYPE_INT32,
        TYPE_ARRAY_UINT32 = OFFSET_ARRAY_FLAG + TYPE_UINT32,
        TYPE_ARRAY_INT64 = OFFSET_ARRAY_FLAG + TYPE_INT64,
        TYPE_ARRAY_UINT64 = OFFSET_ARRAY_FLAG + TYPE_UINT64,
        TYPE_ARRAY_FLOAT = OFFSET_ARRAY_FLAG + TYPE_FLOAT,
        TYPE_ARRAY_DOUBLE = OFFSET_ARRAY_FLAG + TYPE_DOUBLE,
        TYPE_ARRAY_STRING = OFFSET_ARRAY_FLAG + TYPE_STRING,
        TYPE_ARRAY_BYTES = OFFSET_ARRAY_FLAG + TYPE_BYTES,
        TYPE_ARRAY_STRUCT = OFFSET_ARRAY_FLAG + TYPE_STRUCT,
        TYPE_ARRAY_ENUM = OFFSET_ARRAY_FLAG + TYPE_ENUM,
    };

}   // namespace finalmq
