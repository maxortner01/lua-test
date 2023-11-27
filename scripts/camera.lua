function Update(world, entity)
    local player = world:getEntity("PlayerTop")
    local player_transform = player:getComponent(Component.Transform)
    local camera_transform = entity:getComponent(Component.Transform)

    local diff = {
        x = player_transform.position.x - camera_transform.position.x,
        y = player_transform.position.y - camera_transform.position.y
    }

    local factor = 2.5 * Time.deltaTime()
    camera_transform.position.x = camera_transform.position.x + diff.x * factor
    camera_transform.position.y = camera_transform.position.y + diff.y * factor
    camera_transform.rotation = player_transform.rotation
    entity:setComponent(camera_transform)
end