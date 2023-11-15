function Start(world, entity)
    Debug.log("Generating map")
    local tilemap = entity:getComponent(Component.Tilemap)
    
    for y = 10, 15 do
        for x = 10, 15 do
            tilemap:setTile(x, y, x - 10, y - 10)
        end
    end

end