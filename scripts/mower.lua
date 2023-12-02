function Update(world, entity)
    local force = {
        amount = 0,
        mag = 2
    }

    local rotation = {
        amount = 0,
        mag = 2.4
    }

    if Input.getDown("D") then
        rotation.amount = rotation.amount + rotation.mag
    end

    if (Input.getDown("A")) then
        rotation.amount = rotation.amount - rotation.mag
    end

    if (Input.getDown("S")) then
        force.amount = force.amount - force.mag
    end

    if (Input.getDown("W")) then
        force.amount = force.amount + force.mag
    end

    local rigidbody = entity:getComponent(Component.Rigidbody)
    local transform = entity:getComponent(Component.Transform)

    transform.rotation = transform.rotation + rotation.amount * force.amount * 0.1
    rigidbody.addedForce.x = force.amount * math.sin(math.rad(transform.rotation));
    rigidbody.addedForce.y = force.amount * math.cos(math.rad(transform.rotation));
    
    entity:setComponent(rigidbody)
    entity:setComponent(transform)
end