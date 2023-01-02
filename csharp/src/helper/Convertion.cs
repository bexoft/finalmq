using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading;
using System.Diagnostics;

namespace finalmq
{
    public class Convertion
    {
        public static T? Convert<T>(dynamic? from)
        {
            if (from == null)
            {
                return default(T);
            }
            Type typeFrom = from.GetType();
            if (typeof(T) == typeof(string))
            {
                if (typeFrom == typeof(bool))
                {
                    if ((bool)from)
                    {
                        dynamic t = "true";
                        return t;
                    }
                    else
                    {
                        dynamic f = "false";
                        return f;
                    }
                }
                else
                {
                    return System.Convert.ToString(from, System.Globalization.CultureInfo.InvariantCulture);
                }
            }
            else if (typeFrom == typeof(string))
            {
                if (typeof(T) == typeof(bool))
                {
                    if (from != "true" && from != "True")
                    {
                        double value = 0;
                        Double.TryParse(from, System.Globalization.NumberStyles.Any, System.Globalization.CultureInfo.InvariantCulture, out value);
                        if (value == 0)
                        {
                            return (dynamic)false;
                        }
                    }
                    return (dynamic)true;
                }
                else if (typeof(T) == typeof(int))
                {
                    int value;
                    if (Int32.TryParse(from, out value))
                    {
                        return (dynamic)value;
                    }
                    double d = 0;
                    Double.TryParse(from, System.Globalization.NumberStyles.Any, System.Globalization.CultureInfo.InvariantCulture, out d);
                    return (dynamic)(int)d;
                }
                else if (typeof(T) == typeof(uint))
                {
                    uint value;
                    if (UInt32.TryParse(from, System.Globalization.NumberStyles.Any, System.Globalization.CultureInfo.InvariantCulture, out value))
                    {
                        return (dynamic)value;
                    }
                    double d = 0;
                    Double.TryParse(from, System.Globalization.NumberStyles.Any, System.Globalization.CultureInfo.InvariantCulture, out d);
                    return (dynamic)(uint)d;
                }
                else if (typeof(T) == typeof(long))
                {
                    long value;
                    if (Int64.TryParse(from, System.Globalization.NumberStyles.Any, System.Globalization.CultureInfo.InvariantCulture, out value))
                    {
                        return (dynamic)value;
                    }
                    double d = 0;
                    Double.TryParse(from, System.Globalization.NumberStyles.Any, System.Globalization.CultureInfo.InvariantCulture, out d);
                    return (dynamic)(long)d;
                }
                else if (typeof(T) == typeof(ulong))
                {
                    ulong value;
                    if (UInt64.TryParse(from, System.Globalization.NumberStyles.Any, System.Globalization.CultureInfo.InvariantCulture, out value))
                    {
                        return (dynamic)value;
                    }
                    double d = 0;
                    Double.TryParse(from, System.Globalization.NumberStyles.Any, System.Globalization.CultureInfo.InvariantCulture, out d);
                    return (dynamic)(ulong)d;
                }
                else if (typeof(T) == typeof(float))
                {
                    float value;
                    if (Single.TryParse(from, System.Globalization.NumberStyles.Any, System.Globalization.CultureInfo.InvariantCulture, out value))
                    {
                        return (dynamic)value;
                    }
                    double d = 0;
                    Double.TryParse(from, System.Globalization.NumberStyles.Any, System.Globalization.CultureInfo.InvariantCulture, out d);
                    return (dynamic)(float)d;
                }
                else if (typeof(T) == typeof(double))
                {
                    double value;
                    Double.TryParse(from, System.Globalization.NumberStyles.Any, System.Globalization.CultureInfo.InvariantCulture, out value);
                    return (dynamic)value;
                }
            }
            else if (typeof(T) == typeof(bool))
            {
                if (typeFrom == typeof(bool))
                {
                    return from;
                }
                if ((int)from != 0)
                {
                    return (dynamic)true;
                }
                return (dynamic)false;
            }
            else
            {
                if (typeFrom == typeof(bool))
                {
                    from = (from) ? 1 : 0;
                }

                try
                {
                    T value = (T)from;
                    return value;
                }
                catch (Exception)
                {
                }
            }
            return default(T);
        }

        public static dynamic? Convert(dynamic? from, Type typeTo)
        {
            if (from == null)
            {
                return null;
            }
            Type typeFrom = from.GetType();
            if (typeTo == typeof(string))
            {
                if (typeFrom == typeof(bool))
                {
                    if ((bool)from)
                    {
                        dynamic t = "true";
                        return t;
                    }
                    else
                    {
                        dynamic f = "false";
                        return f;
                    }
                }
                else
                {
                    return System.Convert.ToString(from, System.Globalization.CultureInfo.InvariantCulture);
                }
            }
            else if (typeFrom == typeof(string))
            {
                if (typeTo == typeof(bool))
                {
                    if (from != "true" && from != "True")
                    {
                        double value = 0;
                        Double.TryParse(from, System.Globalization.NumberStyles.Any, System.Globalization.CultureInfo.InvariantCulture, out value);
                        if (value == 0)
                        {
                            return (dynamic)false;
                        }
                    }
                    return (dynamic)true;
                }
                else if (typeTo == typeof(int))
                {
                    int value;
                    if (Int32.TryParse(from, out value))
                    {
                        return value;
                    }
                    double d = 0;
                    Double.TryParse((string)from, System.Globalization.NumberStyles.Any, System.Globalization.CultureInfo.InvariantCulture, out d);
                    return (int)d;
                }
                else if (typeTo == typeof(uint))
                {
                    uint value;
                    if (UInt32.TryParse(from, System.Globalization.NumberStyles.Any, System.Globalization.CultureInfo.InvariantCulture, out value))
                    {
                        return value;
                    }
                    double d = 0;
                    Double.TryParse((string)from, System.Globalization.NumberStyles.Any, System.Globalization.CultureInfo.InvariantCulture, out d);
                    return (uint)d;
                }
                else if (typeTo == typeof(long))
                {
                    long value;
                    if (Int64.TryParse(from, System.Globalization.NumberStyles.Any, System.Globalization.CultureInfo.InvariantCulture, out value))
                    {
                        return (dynamic)value;
                    }
                    double d = 0;
                    Double.TryParse((string)from, System.Globalization.NumberStyles.Any, System.Globalization.CultureInfo.InvariantCulture, out d);
                    return (long)d;
                }
                else if (typeTo == typeof(ulong))
                {
                    ulong value;
                    if (UInt64.TryParse(from, System.Globalization.NumberStyles.Any, System.Globalization.CultureInfo.InvariantCulture, out value))
                    {
                        return value;
                    }
                    double d = 0;
                    Double.TryParse((string)from, System.Globalization.NumberStyles.Any, System.Globalization.CultureInfo.InvariantCulture, out d);
                    return (ulong)d;
                }
                else if (typeTo == typeof(float))
                {
                    float value;
                    if (Single.TryParse(from, System.Globalization.NumberStyles.Any, System.Globalization.CultureInfo.InvariantCulture, out value))
                    {
                        return value;
                    }
                    double d = 0;
                    Double.TryParse((string)from, System.Globalization.NumberStyles.Any, System.Globalization.CultureInfo.InvariantCulture, out d);
                    return (float)d;
                }
                else if (typeTo == typeof(double))
                {
                    double value;
                    Double.TryParse(from, System.Globalization.NumberStyles.Any, System.Globalization.CultureInfo.InvariantCulture, out value);
                    return value;
                }
            }
            return null;
        }

        public static int ConvertByteStringToInt32(byte[] buffer, int offset, int size, out bool ok)
        {
            if (size == 0)
            {
                ok = false;
                return 0;
            }

            ok = true;
            int value = 0;

            int i = 0;
            bool neg = false;
            if (buffer[offset] == '-')
            {
                neg = true;
                i = 1;
            }
            for ( ; i < size; i++)
            {
                byte c = buffer[offset + i];
                if (i == 0 && c == '-')
                {
                    neg = true;
                }
                else if (c >= '0' && c <= '9')
                {
                    int diff = (c - '0');
                    if ((value <= 0x0CCCCCCB) || !((value >= 0x0CCCCCCD) || (value == 0x0CCCCCCC && (diff >= 9 || diff == 8 && !neg))))
                    {
                        value *= 10;
                        value += diff;
                    }
                    else
                    {
                        ok = false;
                        break;
                    }
                }
                else
                {
                    ok = false;
                    break;
                }
            }

            if (ok)
            {
                if (neg)
                {
                    value *= -1;
                }
            }
            else
            {
                value = 0;
            }

            return value;
        }

        public static uint ConvertByteStringToUInt32(byte[] buffer, int offset, int size, out bool ok)
        {
            if (size == 0)
            {
                ok = false;
                return 0;
            }

            ok = true;
            uint value = 0;

            for (int i = 0; i < size; i++)
            {
                byte c = buffer[offset + i];
                if (c >= '0' && c <= '9')
                {
                    uint diff = (uint)(c - '0');
                    if ((value <= 0x19999998) || !((value >= 0x1999999A) || (value == 0x19999999 && diff >= 6)))
                    {
                        value *= 10;
                        value += diff;
                    }
                    else
                    {
                        ok = false;
                        break;
                    }
                }
                else
                {
                    ok = false;
                    break;
                }
            }

            if (!ok)
            {
                value = 0;
            }
            return value;
        }

        public static long ConvertByteStringToInt64(byte[] buffer, int offset, int size, out bool ok)
        {
            if (size == 0)
            {
                ok = false;
                return 0;
            }

            ok = true;
            long value = 0;

            int i = 0;
            bool neg = false;
            if (buffer[offset] == '-')
            {
                neg = true;
                i = 1;
            }
            for ( ; i < size; i++)
            {
                byte c = buffer[offset + i];
                if (c >= '0' && c <= '9')
                {
                    int diff = (c - '0');
                    if ((value <= 0x0CCCCCCCCCCCCCCB) || !((value >= 0x0CCCCCCCCCCCCCCD) || (value == 0x0CCCCCCCCCCCCCCC && (diff >= 9 || diff == 8 && !neg))))
                    {
                        value *= 10;
                        value += diff;
                    }
                    else
                    {
                        ok = false;
                        break;
                    }
                }
                else
                {
                    ok = false;
                    break;
                }
            }

            if (ok)
            {
                if (neg)
                {
                    value *= -1;
                }
            }
            else
            {
                value = 0;
            }

            return value;
        }

        public static ulong ConvertByteStringToUInt64(byte[] buffer, int offset, int size, out bool ok)
        {
            if (size == 0)
            {
                ok = false;
                return 0;
            }

            ok = true;
            ulong value = 0;

            for (int i = 0; i < size; i++)
            {
                byte c = buffer[offset + i];
                if (c >= '0' && c <= '9')
                {
                    uint diff = (uint)(c - '0');
                    if ((value <= 0x1999999999999998) || !((value >= 0x199999999999999A) || (value == 0x1999999999999999 && diff >= 6)))
                    {
                        value *= 10;
                        value += diff;
                    }
                    else
                    {
                        ok = false;
                        break;
                    }
                }
                else
                {
                    ok = false;
                    break;
                }
            }
            if (!ok)
            {
                value = 0;
            }
            return value;
        }
    }
}
