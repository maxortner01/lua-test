-- Possibly need a UI state that lives the lifetime of the script
-- That way we can reduce paramteres needed in each call with functions
-- like surface:setFont(...)


function RenderUI(surface)
    --surface:drawText(0, 0, 16, "FPS: "..string.format("%.1f", 1.0 / Time.deltaTime()), "arial");
    surface:drawText(-0.85, 0.5, 22, "Hello,", "arial");
    surface:drawText(-0.85, 0, 22, "I'm writing to you to ask something", "arial");
    surface:drawText(-0.85, -0.1, 22, "This is random text I've just come up with", "arial");

    --[[
    surface:drawTexture(
        "MowCamera",
        ResourceType.Surface,
        {
            position = { x = 0, y = 0 },
            scale = 0.35
        })--]]
end