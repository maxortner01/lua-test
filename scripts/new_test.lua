function Start(entity)
    local rigid_body = entity:getComponent(Component.Rigidbody)
    Debug.assert(rigid_body.good)

    rigid_body.velocity.x = 10
    rigid_body.velocity.y = 0

    entity:setComponent(rigid_body)
end

function Update(entity)
    local position   = entity:getComponent(Component.Position)
    local rigid_body = entity:getComponent(Component.Rigidbody) 
    Debug.assert(position.good)
    Debug.assert(rigid_body.good)

    Debug.log("Position = ("..position.x..", "..position.y..")")

    local dt = 0.001
    position.x = position.x + rigid_body.velocity.x * dt
    position.y = position.y + rigid_body.velocity.y * dt

    Debug.assert(entity:setComponent(position))
    Debug.assert(entity:setComponent(rigid_body))
end