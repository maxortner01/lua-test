function Update(world, entity)
    local player_top = world:getEntity("PlayerTop")
    local transform = player_top:getComponent(Component.Transform)
    transform.position.z = 1
    entity:setComponent(transform)
end