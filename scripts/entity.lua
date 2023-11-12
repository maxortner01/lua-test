function Check(object)
    if not object.good then
        Debug.error(object.what)
        Debug.assert(object.good)
    end
end

function Update(entity)
    local rigid_body = entity:getComponent(Component.RigidBody)
    Check(rigid_body)

    -- Add the gravity force to the velocity (should use metatables later to utilize __add)
    --rigid_body.velocity.y = rigid_body.velocity.y + 200.0 * Time.deltaTime()

    Debug.log("Velocity = ("..rigid_body.velocity.x..", "..rigid_body.velocity.y..")")

    -- Get the Position
    local position = entity:getComponent(Component.Position)
    Check(position)

    -- Add to the position the velocity
    --position.x = position.x + rigid_body.velocity.x * Time.deltaTime()
    --position.y = position.y + rigid_body.velocity.y * Time.deltaTime()

    position.x = 100
    Debug.log("Position = ("..position.x..", "..position.y..")")

    -- Set the velocity and set the position
    Debug.assert(entity:setComponent(position))
    Debug.assert(entity:setComponent(rigid_body))
end