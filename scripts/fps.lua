function Update(world, entity)
    local text = entity:getComponent(Component.Text)
    Debug.assert(text.good)

    text.string = "FPS: "..string.format("%.1f", 1.0 / Time.deltaTime())
    entity:setComponent(text)
end


