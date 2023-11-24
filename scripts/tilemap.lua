function Start(world, entity)
    Debug.log("Generating map")
    local tilemap = entity:getComponent(Component.Tilemap)
    tilemap:setLayerState(1, LayerState.Solid)
    
    local floor       = { x = 2, y = 2 }
    local top_wall    = { x = 1, y = 0 }
    local left_wall   = { x = 0, y = 1 }
    local right_wall  = { x = 5, y = 1 }
    local bottom_wall = { x = 1, y = 4 }

    local size = 15
    for y = 0, size do
        for x = 0, size do
            if x == 0 and y < size then
                tilemap:setTile(1, x, y, left_wall.x, left_wall.y)
            elseif y == 0 and x < size then
                tilemap:setTile(1, x, y, top_wall.x, top_wall.y)
            elseif y == size and x > 0 and x < size then
                tilemap:setTile(1, x, y, bottom_wall.x, bottom_wall.y)
            elseif x == size and y < size then
                tilemap:setTile(1, x, y, right_wall.x, right_wall.y)
            elseif x > 0 and x < size and y > 0 and y < size then
                tilemap:setTile(0, x, y, floor.x, floor.y)
            elseif x == 0 and y == size then
                tilemap:setTile(1, x, y, 0, 4)
            elseif x == size and y == size then
                tilemap:setTile(1, x, y, 5, 4)
            end
        end
    end
end