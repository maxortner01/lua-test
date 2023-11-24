-- Possibly need a UI state that lives the lifetime of the script
-- That way we can reduce paramteres needed in each call with functions
-- like surface:setFont(...)


function RenderUI(surface)
    surface:drawText(0, 0, 16, "FPS: "..string.format("%.1f", 1.0 / Time.deltaTime()), "arial");
end