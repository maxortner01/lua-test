function Update(world, entity)
    if Input.getPressed("LeftClick") ~= true then
        return;
    end

    local mouse_pos = Input.getMousePosition()
    local transform = entity:getComponent(Component.Transform)

    local diff = {
        x = mouse_pos.x - transform.position.x,
        y = mouse_pos.y - transform.position.y
    }
    diff = Math.normalize(diff)

    local bullet = world:createEntity(Component.Transform, Component.Rigidbody, Component.Sprite, Component.Collider)

    local bullet_collider  = bullet:getComponent(Component.Collider)
    local bullet_sprite    = bullet:getComponent(Component.Sprite)
    local bullet_transform = bullet:getComponent(Component.Transform)
    local bullet_rigidbody = bullet:getComponent(Component.Rigidbody)

    -- Add script "bullet", when bullet collides destroy it
    bullet:addScript(Directory.Source.."/scripts/bullet.lua")

    bullet_collider.ColliderComponent = Component.Sprite

    bullet_sprite.size.width  = 10
    bullet_sprite.size.height = 10

    local distance = 50.0
    bullet_transform.position.x = transform.position.x + distance * diff.x
    bullet_transform.position.y = transform.position.y + distance * diff.y
    bullet_transform.position.z = transform.position.z

    bullet_rigidbody.velocity.x = diff.x * 1000.0
    bullet_rigidbody.velocity.y = diff.y * 1000.0

    bullet:setComponent(bullet_collider)
    bullet:setComponent(bullet_transform)
    bullet:setComponent(bullet_rigidbody)
    bullet:setComponent(bullet_sprite)
end