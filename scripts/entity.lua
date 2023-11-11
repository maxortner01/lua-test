Frame = 0

function Check(object)
    if not object.good then
        Debug.log(object.what)
        Debug.assert(object.good)
    end
end

function Start()
    --Debug.log("Hello")
end

function Update(entity)
    local rigid_body = entity:getComponent("RigidBody")
    Check(rigid_body)

    -- Add the gravity force to the velocity (should use metatables later to utilize __add)
    rigid_body.velocity.y = rigid_body.velocity.y + 200.0 * Time.deltaTime()

    -- Debug.log("Velocity = ("..rigid_body.velocity.x..", "..rigid_body.velocity.y..")")

    -- Get the Position
    local position = entity:getComponent("Position")
    Check(position)

    -- Add to the position the velocity
    position.x = position.x + rigid_body.velocity.x * Time.deltaTime()
    position.y = position.y + rigid_body.velocity.y * Time.deltaTime()

    -- Debug.log("Position = ("..position.x..", "..position.y..")")

    -- Set the velocity and set the position
    Debug.assert(entity:setComponent(position))
    Debug.assert(entity:setComponent(rigid_body))
end