#include "pch.h"
#include "InputLayouts.h"

namespace Render
{

    bool operator<(const shader_input_desc& l, const shader_input_desc& r)
    {
        OP(name,
           OP(index, return false;
             )
          );
    }


    bool operator==(const shader_input_desc& l, const shader_input_desc& r)
    {
        return l.name == r.name && l.index == r.index;
    }

    bool operator<(const shader_inputs& l, const shader_inputs& r)
    {
        if (l.inputs.size() == r.inputs.size())
        {
            for (unsigned int i = 0; i < l.inputs.size(); i++)
                if (!(l.inputs[i] == r.inputs[i]))
                    return l.inputs[i] < r.inputs[i];

            return false;
        }

        return l.inputs.size() < r.inputs.size();
    }

    bool operator==(const shader_inputs& l, const shader_inputs& r)
    {
        if (l.inputs.size() == r.inputs.size())
        {
            for (unsigned int i = 0; i < l.inputs.size(); i++)
            {
                if (l.inputs[i].name == r.inputs[i].name)
                    return l.inputs[i].index == r.inputs[i].index;
                else
                    return false;
            }

            return true;
        }

        else
            return false;
    }


    bool operator<(const vertex_buffer_layout& l, const vertex_buffer_layout& r)
    {
        if (l.name == r.name)
        {
            if (l.index == r.index)
            {
                if (l.slot == r.slot)
                {
                    if (l.offset == r.offset)
                    {
                        if (l.format == r.format)
                            return false;
                        else
                            return l.format < r.format;
                    }

                    else
                        return l.offset < r.offset;
                }

                else
                    return l.slot < r.slot;
            }

            else
                return l.index < r.index;
        }

        else
            return l.name < r.name;
    }

}