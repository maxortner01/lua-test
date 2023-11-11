#pragma once

namespace S2D::Util
{

    struct NoCopy
    {
        NoCopy() = default;
        virtual ~NoCopy() = default;
        
        NoCopy(const NoCopy&) = delete;
    };

} // S2D::Util