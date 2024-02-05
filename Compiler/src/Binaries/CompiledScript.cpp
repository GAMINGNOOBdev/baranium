#include "CompiledScript.h"

namespace Binaries
{

    /**
     * @brief Construct a new `CompiledScript` object
     */
    CompiledScript::CompiledScript()
    {
    }

    /**
     * @brief Load a compiled script from
     * 
     * @param file The path to the compiled file
     */
    void CompiledScript::Load(BgeFile& file)
    {
        if (!file.Ready())
            return;

    }

    /**
     * @brief Write a compiled script
     * 
     * @param file The destination path to the compiled file
     */
    void CompiledScript::Save(BgeFile& file)
    {
        if (!file.Ready())
            return;

    }

}