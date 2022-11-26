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
        public static T Convert<T>(dynamic from)
        {
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
                    if (from == "true" || from == "True")
                    {
                        return (dynamic)true;
                    }
                    else
                    {
                        return (dynamic)false;
                    }
                }
                else if (typeof(T) == typeof(int))
                {
                    int value;
                    if (Int32.TryParse(from, out value))
                    {
                        return (dynamic)value;
                    }
                }
                else if (typeof(T) == typeof(uint))
                {
                    uint value;
                    if (UInt32.TryParse(from, out value))
                    {
                        return (dynamic)value;
                    }
                }
                else if (typeof(T) == typeof(long))
                {
                    long value;
                    if (Int64.TryParse(from, out value))
                    {
                        return (dynamic)value;
                    }
                }
                else if (typeof(T) == typeof(ulong))
                {
                    ulong value;
                    if (UInt64.TryParse(from, out value))
                    {
                        return (dynamic)value;
                    }
                }
                else if (typeof(T) == typeof(float))
                {
                    double value;
                    if (Double.TryParse(from, out value))
                    {
                        return (dynamic)(float)value;
                    }
                }
                else if (typeof(T) == typeof(double))
                {
                    double value;
                    if (Double.TryParse(from, out value))
                    {
                        return (dynamic)value;
                    }
                }
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
                catch (Exception ex)
                {
                }
            }
            return default(T);
        }
    }
}
