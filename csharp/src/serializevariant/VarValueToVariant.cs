using System;
using System.Collections.Generic;
using System.Diagnostics;


namespace finalmq
{
    using VariantStruct = List<NameValue>;
    using VariantList = List<Variant>;

    public enum VarValueType2Index
    {
        VARVALUETYPE_NONE = 0,
        VARVALUETYPE_BOOL = 1,
        VARVALUETYPE_INT8 = 2,
        VARVALUETYPE_UINT8 = 3,
        VARVALUETYPE_INT16 = 4,
        VARVALUETYPE_UINT16 = 5,
        VARVALUETYPE_INT32 = 6,
        VARVALUETYPE_UINT32 = 7,
        VARVALUETYPE_INT64 = 8,
        VARVALUETYPE_UINT64 = 9,
        VARVALUETYPE_FLOAT = 10,
        VARVALUETYPE_DOUBLE = 11,
        VARVALUETYPE_STRING = 12,
        VARVALUETYPE_BYTES = 13,
        VARVALUETYPE_VARIANTSTRUCT = 14,
        VARVALUETYPE_ARRAY_BOOL = 15,
        VARVALUETYPE_ARRAY_INT8 = 16,
        VARVALUETYPE_ARRAY_INT16 = 17,
        VARVALUETYPE_ARRAY_UINT16 = 18,
        VARVALUETYPE_ARRAY_UINT32 = 19,
        VARVALUETYPE_ARRAY_INT32 = 20,
        VARVALUETYPE_ARRAY_INT64 = 21,
        VARVALUETYPE_ARRAY_UINT64 = 22,
        VARVALUETYPE_ARRAY_FLOAT = 23,
        VARVALUETYPE_ARRAY_DOUBLE = 24,
        VARVALUETYPE_ARRAY_STRING = 25,
        VARVALUETYPE_ARRAY_BYTES = 26,
        VARVALUETYPE_VARIANTLIST = 27,
    }

    public class VarValueToVariant
    {
        public delegate void FuncExit();
        public VarValueToVariant(Variant variant)
        {
            m_variant = variant;
            m_varValue = new variant.VarValue();
            m_serializerStruct = new SerializerStruct(m_varValue);
        }

        public IParserVisitor GetVisitor()
        {
            return m_serializerStruct;
        }

        public void SetExitNotification(FuncExit? funcExit)
        {
            m_serializerStruct.SetExitNotification(funcExit);
        }
        public void Convert()
        {
            ProcessVarValue(m_varValue, m_variant);
        }

        void ProcessVarValue(variant.VarValue varValue, Variant var)
        {
            switch ((VarValueType2Index)varValue.index)
            {
                case VarValueType2Index.VARVALUETYPE_NONE:
                    break;
                case VarValueType2Index.VARVALUETYPE_BOOL:
                    var.SetData(varValue.valbool);
                    break;
                case VarValueType2Index.VARVALUETYPE_INT8:
                    var.SetData(varValue.valint8);
                    break;
                case VarValueType2Index.VARVALUETYPE_UINT8:
                    var.SetData(varValue.valuint8);
                    break;
                case VarValueType2Index.VARVALUETYPE_INT16:
                    var.SetData(varValue.valint16);
                    break;
                case VarValueType2Index.VARVALUETYPE_UINT16:
                    var.SetData(varValue.valuint16);
                    break;
                case VarValueType2Index.VARVALUETYPE_INT32:
                    var.SetData(varValue.valint32);
                    break;
                case VarValueType2Index.VARVALUETYPE_UINT32:
                    var.SetData(varValue.valuint32);
                    break;
                case VarValueType2Index.VARVALUETYPE_INT64:
                    var.SetData(varValue.valint64);
                    break;
                case VarValueType2Index.VARVALUETYPE_UINT64:
                    var.SetData(varValue.valuint64);
                    break;
                case VarValueType2Index.VARVALUETYPE_FLOAT:
                    var.SetData(varValue.valfloat);
                    break;
                case VarValueType2Index.VARVALUETYPE_DOUBLE:
                    var.SetData(varValue.valdouble);
                    break;
                case VarValueType2Index.VARVALUETYPE_STRING:
                    var.SetData(varValue.valstring);
                    break;
                case VarValueType2Index.VARVALUETYPE_BYTES:
                    var.SetData(varValue.valbytes);
                    break;
                case VarValueType2Index.VARVALUETYPE_VARIANTSTRUCT:
                    VariantStruct variantStruct = new VariantStruct();
                    Debug.Assert(variantStruct != null);
                    foreach (var varValueElement in varValue.valstruct)
                    {
                        Variant v = new Variant();
                        ProcessVarValue(varValueElement, v);
                        variantStruct.Add(new NameValue(varValueElement.name, v));
                    }
                    var.SetData(variantStruct);
                    break;

                case VarValueType2Index.VARVALUETYPE_ARRAY_BOOL:
                    var.SetData(varValue.valarrbool);
                    break;
                case VarValueType2Index.VARVALUETYPE_ARRAY_INT8:
                    var.SetData(varValue.valarrint8);
                    break;
                case VarValueType2Index.VARVALUETYPE_ARRAY_INT16:
                    var.SetData(varValue.valarrint16);
                    break;
                case VarValueType2Index.VARVALUETYPE_ARRAY_UINT16:
                    var.SetData(varValue.valarruint16);
                    break;
                case VarValueType2Index.VARVALUETYPE_ARRAY_INT32:
                    var.SetData(varValue.valarrint32);
                    break;
                case VarValueType2Index.VARVALUETYPE_ARRAY_UINT32:
                    var.SetData(varValue.valarruint32);
                    break;
                case VarValueType2Index.VARVALUETYPE_ARRAY_INT64:
                    var.SetData(varValue.valarrint64);
                    break;
                case VarValueType2Index.VARVALUETYPE_ARRAY_UINT64:
                    var.SetData(varValue.valarruint64);
                    break;
                case VarValueType2Index.VARVALUETYPE_ARRAY_FLOAT:
                    var.SetData(varValue.valarrfloat);
                    break;
                case VarValueType2Index.VARVALUETYPE_ARRAY_DOUBLE:
                    var.SetData(varValue.valarrdouble);
                    break;
                case VarValueType2Index.VARVALUETYPE_ARRAY_STRING:
                    var.SetData(varValue.valarrstring);
                    break;
                case VarValueType2Index.VARVALUETYPE_ARRAY_BYTES:
                    var.SetData(varValue.valarrbytes);
                    break;
                case VarValueType2Index.VARVALUETYPE_VARIANTLIST:
                    VariantList variantList = new VariantList();
                    foreach (var varValueElement in varValue.vallist)
                    {
                        Variant v = new Variant();
                        ProcessVarValue(varValueElement, v);
                        variantList.Add(v);
                    }
                    var.SetData(variantList);
                    break;
                default:
                    break;
            }
        }

        readonly Variant m_variant;
        readonly SerializerStruct m_serializerStruct;
        readonly variant.VarValue m_varValue;
    };

}
