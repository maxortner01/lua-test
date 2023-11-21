function Start(world, entity)
    local rigidbody = entity:getComponent(Component.Rigidbody)

    rigidbody.linearDrag = 10.0

    entity:setComponent(rigidbody)
end

function Update(world, entity)
    local force = {
        x = 0, y = 0
    }

    if Input.getDown("D") then
        force.x = force.x + 1
    end

    if (Input.getDown("A")) then
        force.x = force.x - 1
    end

    if (Input.getDown("S")) then
        force.y = force.y + 1
    end

    if (Input.getDown("W")) then
        force.y = force.y - 1
    end

    local rigidbody = entity:getComponent(Component.Rigidbody)

    local magnitude = 4000
    force = Math.normalize(force)
    force.x = force.x * magnitude
    force.y = force.y * magnitude

    rigidbody.addedForce.x = force.x;
    rigidbody.addedForce.y = force.y;
    
    entity:setComponent(rigidbody)
end