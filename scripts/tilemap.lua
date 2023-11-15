function Update(world, entity)
    Debug.log("Running")
    local tilemap = entity:getComponent(Component.Tilemap)
    Debug.assert(tilemap:setTile())
end