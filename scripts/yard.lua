function Start(world, entity)
    local tilemap = entity:getComponent(Component.Tilemap)

    local size = 20
    for y = 0, size - 1 do
        for x = 0, size - 1 do
            tilemap:setTile(0, x, y, 0, 0)
        end
    end
end