
function Update(entity)
    local position   = entity:getComponent(Component.Position)
    local rigid_body = entity:getComponent(Component.RigidBody) 
    Debug.assert(position.good)
    Debug.assert(rigid_body.good)

    Debug.log("Position = ("..position.x..", "..position.y..")")

    position.x = 520
    Debug.assert(entity:setComponent(position))
    Debug.assert(entity:setComponent(rigid_body))
end