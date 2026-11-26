namespace NBodySim;

internal enum CollisionMode
{
    Static,
    Dynamic
}

internal sealed class SimBody
{
    public double[] Position { get; } = new double[3];
    public double[] Velocity { get; } = new double[3];
    public double Mass { get; set; }
    public double Radius { get; set; }
    public bool Collision { get; set; }
    public bool Ignore { get; set; }
}

internal sealed class BodySimulation
{
    private const double GravitationalConstant = -6.670e-11;

    public List<SimBody> Bodies { get; private set; } = [];
    public double TimeFactor { get; private set; } = 100;
    public double Restitution { get; set; } = 0.5;
    public int ViewportDistance { get; set; } = 1000;
    public bool LockViewport { get; set; }
    public bool RandomInitialVelocities { get; set; }
    public bool ThreeDimensions { get; set; }
    public CollisionMode CollisionMode { get; set; } = CollisionMode.Static;
    public int Frame { get; private set; }
    public int ViewableCount { get; private set; }
    public double Left { get; private set; } = -300;
    public double Right { get; private set; } = 300;
    public double Top { get; private set; } = -240;
    public double Bottom { get; private set; } = 240;
    public double CenterX { get; private set; }
    public double CenterY { get; private set; }
    public double CenterZ { get; private set; }
    public double Close { get; private set; } = -300;
    public double Distant { get; private set; } = 300;

    public BodySimulation()
    {
        Reset(2);
    }

    public void Reset(int count)
    {
        Bodies = new List<SimBody>(count);
        Frame = 0;
        for (var i = 0; i < count; i++)
        {
            var mass = Random.Shared.NextDouble() * 100_000;
            var body = new SimBody
            {
                Mass = Math.Max(1, mass),
                Radius = Math.Log(Math.Max(1, mass))
            };
            for (var axis = 0; axis < 3; axis++)
            {
                body.Position[axis] = Random.Shared.NextDouble() * 400 - 200;
            }
            if (RandomInitialVelocities)
            {
                body.Velocity[0] = Random.Shared.NextDouble() * 2 - 1;
                body.Velocity[1] = Random.Shared.NextDouble() * 2 - 1;
                body.Velocity[2] = Random.Shared.NextDouble() * 6 - 3;
            }
            Bodies.Add(body);
        }
        ViewableCount = count;
        RecomputeViewport();
    }

    public void SetTimeFactor(int factor)
    {
        var ratio = factor / TimeFactor;
        foreach (var body in Bodies)
        {
            for (var axis = 0; axis < 3; axis++)
            {
                body.Velocity[axis] *= ratio;
            }
        }
        TimeFactor = factor;
    }

    public void RecomputeViewport()
    {
        var half = ViewportDistance / 2.0;
        Left = Top = Close = -half;
        Right = Bottom = Distant = half;
        CenterX = CenterY = CenterZ = 0;
    }

    public void SetViewportBounds(double left, double top, double right, double bottom)
    {
        Left = left;
        Top = top;
        Right = right;
        Bottom = bottom;
        CenterX = (left + right) / 2;
        CenterY = (top + bottom) / 2;
    }

    public void IgnoreAll(bool ignore)
    {
        foreach (var body in Bodies)
        {
            body.Ignore = ignore;
        }
    }

    public void KillVelocities(int? index = null)
    {
        var start = index ?? 0;
        var end = index.HasValue ? Math.Min(start + 1, Bodies.Count) : Bodies.Count;
        for (var i = start; i < end; i++)
        {
            Array.Clear(Bodies[i].Velocity);
        }
    }

    public bool Step()
    {
        Frame++;
        foreach (var body in Bodies)
        {
            body.Collision = false;
        }

        FlagOutsideViewport();
        ApplyGravity();
        var collided = ResolveLegacyCollisions();
        FitViewport();
        return collided;
    }

    private void FlagOutsideViewport()
    {
        if (ViewportDistance <= 0)
        {
            return;
        }

        foreach (var body in Bodies)
        {
            if (Math.Abs(body.Position[0]) > ViewportDistance ||
                Math.Abs(body.Position[1]) > ViewportDistance ||
                (ThreeDimensions && Math.Abs(body.Position[2]) > ViewportDistance))
            {
                body.Ignore = true;
            }
        }
    }

    private void ApplyGravity()
    {
        var dimensions = ThreeDimensions ? 3 : 2;
        var acceleration = new double[Bodies.Count, dimensions];

        for (var a = 0; a < Bodies.Count; a++)
        {
            for (var b = 0; b < Bodies.Count; b++)
            {
                if (a == b)
                {
                    continue;
                }

                var distanceSquared = 0.0;
                for (var axis = 0; axis < dimensions; axis++)
                {
                    var delta = Bodies[a].Position[axis] - Bodies[b].Position[axis];
                    distanceSquared += delta * delta;
                }
                if (distanceSquared <= double.Epsilon)
                {
                    continue;
                }

                var distance = Math.Sqrt(distanceSquared);
                var denominator = distanceSquared * distance;
                var force = GravitationalConstant * Bodies[b].Mass / denominator;
                for (var axis = 0; axis < dimensions; axis++)
                {
                    acceleration[a, axis] += force *
                        (Bodies[a].Position[axis] - Bodies[b].Position[axis]);
                }
            }
        }

        for (var i = 0; i < Bodies.Count; i++)
        {
            for (var axis = 0; axis < dimensions; axis++)
            {
                Bodies[i].Velocity[axis] += acceleration[i, axis] * TimeFactor;
            }
        }
    }

    private bool ResolveLegacyCollisions()
    {
        var dimensions = ThreeDimensions ? 3 : 2;
        var collisions = 0;

        for (var a = 0; a < Bodies.Count; a++)
        {
            for (var b = 0; b < Bodies.Count; b++)
            {
                if (a == b)
                {
                    continue;
                }

                var first = Bodies[a];
                var second = Bodies[b];
                var delta = new double[dimensions];
                var distanceSquared = 0.0;
                for (var axis = 0; axis < dimensions; axis++)
                {
                    delta[axis] = first.Position[axis] - second.Position[axis];
                    distanceSquared += delta[axis] * delta[axis];
                }

                var radiusSum = first.Radius + second.Radius;
                if (Math.Sqrt(distanceSquared) < radiusSum && radiusSum > 0)
                {
                    collisions++;
                    first.Collision = true;
                    var inverseMassSum = 1 / first.Mass + 1 / second.Mass;
                    for (var axis = 0; axis < dimensions; axis++)
                    {
                        if (delta[axis] == 0)
                        {
                            delta[axis] = 0.0001;
                        }
                        var normal = delta[axis] / radiusSum;
                        var perpendicular = normal * delta[axis];
                        var relativeVelocity = first.Velocity[axis] - second.Velocity[axis];
                        var denominator = perpendicular * perpendicular * inverseMassSum;
                        if (denominator == 0)
                        {
                            continue;
                        }

                        var impulse = -(1 + Restitution) * relativeVelocity *
                                      perpendicular / denominator;
                        first.Velocity[axis] = impulse / first.Mass * perpendicular;
                        second.Velocity[axis] = -impulse / second.Mass * perpendicular;
                    }
                    for (var axis = 0; axis < dimensions; axis++)
                    {
                        first.Position[axis] += first.Velocity[axis];
                        second.Position[axis] += second.Velocity[axis];
                    }
                }
                else
                {
                    for (var axis = 0; axis < dimensions; axis++)
                    {
                        first.Position[axis] += first.Velocity[axis];
                    }
                }
            }
        }

        return collisions > 0;
    }

    private void FitViewport()
    {
        if (!LockViewport)
        {
            var visible = Bodies.Where(body => !body.Ignore).ToArray();
            if (visible.Length > 0)
            {
                Left = visible.Min(body => body.Position[0] - body.Radius);
                Right = visible.Max(body => body.Position[0] + body.Radius);
                Top = visible.Min(body => body.Position[1] - body.Radius);
                Bottom = visible.Max(body => body.Position[1] + body.Radius);
                CenterX = (Left + Right) / 2;
                CenterY = (Top + Bottom) / 2;
                if (ThreeDimensions)
                {
                    Close = visible.Min(body => body.Position[2] - body.Radius);
                    Distant = visible.Max(body => body.Position[2] + body.Radius);
                    CenterZ = (Close + Distant) / 2;
                }
            }
        }

        ViewableCount = Bodies.Count(body => !body.Ignore);
    }

    public void SetMass(int index, double multiplier)
    {
        var body = Bodies[index];
        body.Mass *= multiplier;
        body.Radius = Math.Log(Math.Abs(body.Mass));
    }
}
