function Update(world, entity)
    local velocity = {
        x = 0, y = 0, z = 0
    }

    if Input.getDown("D") then
        velocity.x = velocity.x + 1
    end

    if (Input.getDown("A")) then
        velocity.x = velocity.x - 1
    end

    if (Input.getDown("S")) then
        velocity.y = velocity.y + 1
    end

    if (Input.getDown("W")) then
        velocity.y = velocity.y - 1
    end

    local rigidbody = entity:getComponent(Component.Rigidbody)
    
    local magnitude = 200
    velocity = Math.normalize(velocity)
    velocity.x = velocity.x * magnitude
    velocity.y = velocity.y * magnitude
    velocity.z = velocity.z * magnitude

    rigidbody.velocity.x = rigidbody.velocity.x + velocity.x * Time.deltaTime()
    rigidbody.velocity.y = rigidbody.velocity.y + velocity.y * Time.deltaTime()

    --transform.position.x = transform.position.x + velocity.x * Time.deltaTime()
    --transform.position.y = transform.position.y + velocity.y * Time.deltaTime()
    --transform.position.z = transform.position.z + velocity.z * Time.deltaTime()
    entity:setComponent(rigidbody)
end