function Update(world, entity)
    local player_top = world:getEntity("PlayerTop")
    local transform = player_top:getComponent(Component.Transform)
    transform.position.z = -0.001
    entity:setComponent(transform)
end