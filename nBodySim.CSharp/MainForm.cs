using System.Drawing.Drawing2D;
using System.Globalization;
using System.Media;

namespace NBodySim;

internal sealed class MainForm : Form
{
    private readonly BodySimulation _simulation = new();
    private readonly RenderPanel _canvas = new();
    private readonly System.Windows.Forms.Timer _timer = new();
    private readonly List<(ToolStripMenuItem Item, Func<bool> IsChecked)> _checkItems = [];
    private readonly Dictionary<Keys, Action> _shortcuts = [];
    private readonly Icon _applicationIcon = new(Path.Combine(AppContext.BaseDirectory, "nBodySim.ico"));
    private readonly SoundPlayer _collisionSound = new(Path.Combine(AppContext.BaseDirectory, "collision.wav"));
    private readonly SoundPlayer _aboutSound = new(Path.Combine(AppContext.BaseDirectory, "about.wav"));
    private int _selectedBody;
    private bool _paused;
    private bool _showGrid = true;
    private bool _showNumericData;
    private bool _soundEffects = true;
    private bool _draggingMomentum;
    private Point _momentumStart;
    private Point _momentumEnd;
    private int _soundDelay;

    public MainForm()
    {
        Text = "nBodySim";
        Icon = _applicationIcon;
        ClientSize = new Size(1280, 720);
        KeyPreview = true;

        var menu = BuildMenu();
        MainMenuStrip = menu;
        Controls.Add(_canvas);
        Controls.Add(menu);
        menu.Dock = DockStyle.Top;
        _canvas.Dock = DockStyle.Fill;
        _canvas.BackColor = SystemColors.Window;
        _canvas.Paint += PaintWorld;
        _canvas.MouseDown += CanvasMouseDown;
        _canvas.MouseMove += CanvasMouseMove;
        _canvas.MouseUp += CanvasMouseUp;
        _canvas.MouseWheel += CanvasMouseWheel;
        _canvas.MouseEnter += (_, _) => _canvas.Focus();

        _timer.Interval = 33;
        _timer.Tick += (_, _) => AdvanceSimulation();
        _timer.Start();
        UpdateWindowState();
    }

    private MenuStrip BuildMenu()
    {
        var menu = new MenuStrip();

        var app = AddMenu(menu, "&App");
        AddItem(app, "Pause/Resume", () => _paused = !_paused, Keys.Space);
        var fps = AddMenu(app, "FPS");
        AddChoice(fps, "FPS 1", () => _timer.Interval = 1000, () => _timer.Interval == 1000);
        AddChoice(fps, "FPS 30", () => _timer.Interval = 33, () => _timer.Interval == 33);
        AddChoice(fps, "FPS 60", () => _timer.Interval = 16, () => _timer.Interval == 16);
        app.DropDownItems.Add(new ToolStripSeparator());
        AddItem(app, "Exit", Close);

        var scenarios = AddMenu(menu, "&Scenarios");
        AddItem(scenarios, "2-body collision, 1 static 1 moving", () => LoadScenario(
            (1e3, 0, 0, 0, 0), (1e4, 100, 0, -1, 0)));
        AddItem(scenarios, "2-body equal collision", () => LoadScenario(
            (1e3, 0, 0, 0, 0), (1e3, 100, 0, -1, 0)));
        AddItem(scenarios, "2-body equal collide at 1m each", () => LoadScenario(
            (1e3, -100, 0, 1, 0), (1e3, 100, 0, -1, 0)));
        AddItem(scenarios, "2-body collision at 45 degree angle", () => LoadScenario(
            (1e4, 0, 0, 0, 0), (1e4, 100, 100, -1, -0.8)));
        AddItem(scenarios, "2-body orbit", () => LoadScenario(
            (1e6, -30, 0, 0, -0.025), (1e6, 30, 0, 0, 0.025)));
        scenarios.DropDownItems.Add(new ToolStripSeparator());
        AddItem(scenarios, "3-body collision", () => LoadScenario(
            (1e2, -100, 0, 0, 0), (1e5, 0, 0, 0, 0), (1e3, 100, 0, -1, 0)));
        AddItem(scenarios, "3-body reverse collision", () => LoadScenario(
            (1e2, -100, 0, 1, 0), (1e4, 0, 0, 0, 0), (1e3, 100, 0, 0, 0)));
        scenarios.DropDownItems.Add(new ToolStripSeparator());
        AddItem(scenarios, "Sol System", LoadSolarSystem);

        var viewport = AddMenu(menu, "View &Port");
        AddViewportChoice(viewport, "View 100 distance", 100, Keys.D1);
        AddViewportChoice(viewport, "View 200 distance", 200, Keys.D2);
        AddViewportChoice(viewport, "View 500 distance", 500, Keys.D3);
        AddViewportChoice(viewport, "View 1,000 distance", 1000, Keys.D4);
        AddViewportChoice(viewport, "View 5,000 distance", 5000, Keys.D5);
        AddViewportChoice(viewport, "View 10,000 distance", 10000, Keys.D6);
        viewport.DropDownItems.Add(new ToolStripSeparator());
        AddItem(viewport, "Autozoom", () => SetViewport(0, false));
        AddItem(viewport, "Autozoom - Ignore beyond 100 distance", () => SetViewport(100, false));
        AddItem(viewport, "Autozoom - Ignore beyond 200 distance", () => SetViewport(200, false));
        AddItem(viewport, "Autozoom - Ignore beyond 1,000 distance", () => SetViewport(1000, false));
        AddItem(viewport, "Autozoom - Ignore beyond 5,000 distance", () => SetViewport(5000, false));
        AddItem(viewport, "Autozoom - Ignore beyond 10,000 distance", () => SetViewport(10000, false));
        viewport.DropDownItems.Add(new ToolStripSeparator());
        AddItem(viewport, "Unignore All", () => _simulation.IgnoreAll(false));
        AddItem(viewport, "Ignore All", () => _simulation.IgnoreAll(true));
        AddToggle(viewport, "Lock Viewport", () => _simulation.LockViewport,
            value => _simulation.LockViewport = value, Keys.Scroll);
        AddToggle(viewport, "Numeric Body Data", () => _showNumericData,
            value => _showNumericData = value, Keys.Oemtilde);
        AddToggle(viewport, "Grid", () => _showGrid, value => _showGrid = value, Keys.G);

        var bodies = AddMenu(menu, "nBodies");
        foreach (var count in new[] { 1, 2, 3, 4, 5, 10, 20, 100, 1000, 10000 })
        {
            AddItem(bodies, count.ToString(CultureInfo.InvariantCulture), () => ResetBodies(count));
        }
        bodies.DropDownItems.Add(new ToolStripSeparator());
        var velocities = AddMenu(bodies, "Initial Velocities");
        AddChoice(velocities, "None", () => _simulation.RandomInitialVelocities = false,
            () => !_simulation.RandomInitialVelocities);
        AddChoice(velocities, "Random", () => _simulation.RandomInitialVelocities = true,
            () => _simulation.RandomInitialVelocities);
        bodies.DropDownItems.Add(new ToolStripSeparator());
        AddItem(bodies, "Kill Velocities", () => _simulation.KillVelocities(), Keys.Delete);

        var body = AddMenu(menu, "Body");
        AddItem(body, "Increase Mass", () => ChangeSelectedMass(10), Keys.Add);
        AddItem(body, "Decrease Mass", () => ChangeSelectedMass(0.1), Keys.Subtract);
        body.DropDownItems.Add(new ToolStripSeparator());
        AddToggle(body, "Toggle Ignore", () => Selected.Ignore, value => Selected.Ignore = value, Keys.I);
        AddItem(body, "Kill Velocity", () => _simulation.KillVelocities(_selectedBody), Keys.K);
        body.DropDownItems.Add(new ToolStripSeparator());
        AddItem(body, "Select Next", SelectNext, Keys.Tab);
        AddItem(body, "Select Previous", SelectPrevious, Keys.Shift | Keys.Tab);
        body.DropDownItems.Add(new ToolStripSeparator());
        AddItem(body, "-Z", () => Selected.Position[2] -= (_simulation.Distant - _simulation.Close) / 30,
            Keys.Down);
        AddItem(body, "+Z", () => Selected.Position[2] += (_simulation.Distant - _simulation.Close) / 30,
            Keys.Up);
        body.DropDownItems.Add(new ToolStripSeparator());
        AddItem(body, "Negative Mass", () => Selected.Mass = -Selected.Mass, Keys.M);

        var physics = AddMenu(menu, "Physics Model");
        AddChoice(physics, "Rigid Body", () => { }, () => true);
        var fluid = AddItem(physics, "Fluid Body (unavailable)", () => { });
        fluid.Enabled = false;
        physics.DropDownItems.Add(new ToolStripSeparator());
        AddChoice(physics, "(Classic) Static Collision",
            () => _simulation.CollisionMode = CollisionMode.Static,
            () => _simulation.CollisionMode == CollisionMode.Static);
        AddChoice(physics, "Dynamic Collision",
            () => _simulation.CollisionMode = CollisionMode.Dynamic,
            () => _simulation.CollisionMode == CollisionMode.Dynamic);
        physics.DropDownItems.Add(new ToolStripSeparator());
        AddChoice(physics, "2D", () => _simulation.ThreeDimensions = false,
            () => !_simulation.ThreeDimensions);
        AddChoice(physics, "3D", () => _simulation.ThreeDimensions = true,
            () => _simulation.ThreeDimensions);

        var restitution = AddMenu(menu, "Restitution");
        AddChoice(restitution, "COR=0.0 (clay)", () => _simulation.Restitution = 0,
            () => _simulation.Restitution == 0);
        AddChoice(restitution, "COR=0.5", () => _simulation.Restitution = 0.5,
            () => _simulation.Restitution == 0.5);
        AddChoice(restitution, "COR=1.0 (perfectly elastic)", () => _simulation.Restitution = 1,
            () => _simulation.Restitution == 1);

        var sound = AddMenu(menu, "Sound");
        AddToggle(sound, "Sound Effects", () => _soundEffects, value => _soundEffects = value);

        var help = AddMenu(menu, "&Help");
        AddItem(help, "About nBodySim", ShowAbout);
        return menu;
    }

    private SimBody Selected => _simulation.Bodies[_selectedBody];

    private ToolStripMenuItem AddMenu(ToolStrip menu, string text)
    {
        var item = new ToolStripMenuItem(text);
        menu.Items.Add(item);
        return item;
    }

    private ToolStripMenuItem AddMenu(ToolStripDropDownItem parent, string text)
    {
        var item = new ToolStripMenuItem(text);
        parent.DropDownItems.Add(item);
        return item;
    }

    private ToolStripMenuItem AddItem(ToolStripDropDownItem parent, string text, Action action,
        Keys shortcut = Keys.None)
    {
        var item = new ToolStripMenuItem(text);
        item.Click += (_, _) => action();
        parent.DropDownItems.Add(item);
        if (shortcut != Keys.None)
        {
            _shortcuts[shortcut] = action;
        }
        return item;
    }

    protected override bool ProcessCmdKey(ref Message msg, Keys keyData)
    {
        if (_shortcuts.TryGetValue(keyData, out var action))
        {
            action();
            return true;
        }
        return base.ProcessCmdKey(ref msg, keyData);
    }

    private void AddChoice(ToolStripDropDownItem parent, string text, Action action,
        Func<bool> isSelected)
    {
        var item = AddItem(parent, text, action);
        _checkItems.Add((item, isSelected));
    }

    private void AddToggle(ToolStripDropDownItem parent, string text, Func<bool> getValue,
        Action<bool> setValue, Keys shortcut = Keys.None)
    {
        var item = AddItem(parent, text, () => setValue(!getValue()), shortcut);
        _checkItems.Add((item, getValue));
    }

    private void AddViewportChoice(ToolStripDropDownItem menu, string text, int distance, Keys key)
    {
        AddItem(menu, text, () => SetViewport(distance, true), key);
    }

    private void SetViewport(int distance, bool locked)
    {
        _simulation.IgnoreAll(false);
        _simulation.ViewportDistance = distance;
        _simulation.LockViewport = locked;
        if (locked)
        {
            _simulation.RecomputeViewport();
        }
    }

    private void ResetBodies(int count)
    {
        _simulation.Reset(count);
        _selectedBody = Math.Min(_selectedBody, count - 1);
    }

    private void LoadScenario(params (double Mass, double X, double Y, double Vx, double Vy)[] bodies)
    {
        ResetBodies(bodies.Length);
        for (var i = 0; i < bodies.Length; i++)
        {
            var scenarioBody = bodies[i];
            var body = _simulation.Bodies[i];
            body.Mass = scenarioBody.Mass;
            body.Radius = Math.Log(Math.Abs(scenarioBody.Mass));
            body.Position[0] = scenarioBody.X;
            body.Position[1] = scenarioBody.Y;
            body.Velocity[0] = scenarioBody.Vx;
            body.Velocity[1] = scenarioBody.Vy;
        }

        _simulation.ViewportDistance = 1000;
        _simulation.LockViewport = true;
        _simulation.SetViewportBounds(-200, -200, 200, 200);
        _showNumericData = true;
        _paused = false;
    }

    private void LoadSolarSystem()
    {
        ResetBodies(5);
        var solarBodies = new (double Mass, double Radius, double X)[]
        {
            (1.989e30, Math.Log(1.989e30), 0),
            (3.302e23, 4878, 5.790e10),
            (4.869e24, 12104, 1.082e11),
            (5.974e24, 12756, 1.496e11),
            (6.419e23, 6794, 2.279e11)
        };
        for (var i = 0; i < solarBodies.Length; i++)
        {
            var body = _simulation.Bodies[i];
            body.Mass = solarBodies[i].Mass;
            body.Radius = solarBodies[i].Radius;
            body.Position[0] = solarBodies[i].X;
            body.Position[1] = 0;
            body.Velocity[0] = body.Velocity[1] = 0;
        }
        _simulation.ViewportDistance = 0;
        _simulation.LockViewport = false;
        _simulation.IgnoreAll(false);
        _simulation.RecomputeViewport();
        _showNumericData = true;
        _paused = false;
    }

    private void ChangeSelectedMass(double multiplier)
    {
        _simulation.SetMass(_selectedBody, multiplier);
    }

    private void SelectNext()
    {
        _selectedBody = (_selectedBody + 1) % _simulation.Bodies.Count;
    }

    private void SelectPrevious()
    {
        _selectedBody = (_selectedBody + _simulation.Bodies.Count - 1) % _simulation.Bodies.Count;
    }

    private void AdvanceSimulation()
    {
        if (!_paused)
        {
            if (_simulation.Step() && _soundEffects && _soundDelay <= 0)
            {
                _collisionSound.Play();
                _soundDelay = 500;
            }
            _canvas.Invalidate();
        }
        _soundDelay -= _timer.Interval;
        UpdateWindowState();
    }

    private void UpdateWindowState()
    {
        var distance = _simulation.ViewportDistance == 0
            ? "INFINITE"
            : _simulation.ViewportDistance.ToString("N0", CultureInfo.InvariantCulture);
        Text = $"nBodySim - viewport {distance}{(_simulation.LockViewport ? " [LOCKED]" : "")} - " +
               $"bodies {_simulation.Bodies.Count} ({_simulation.ViewableCount} viewable) - " +
               $"frame {_simulation.Frame}{(_paused ? " [PAUSED]" : "")}";
        foreach (var (item, isChecked) in _checkItems)
        {
            item.Checked = isChecked();
        }
    }

    private void ShowAbout()
    {
        if (_soundEffects)
        {
            _aboutSound.Play();
        }

        using var titleFont = new Font(Font.FontFamily, 15, FontStyle.Bold);
        using var image = Image.FromFile(Path.Combine(
            AppContext.BaseDirectory, "Resources", "sarah.png"));
        using var dialog = new Form
        {
            Text = "About nBodySim",
            FormBorderStyle = FormBorderStyle.FixedDialog,
            StartPosition = FormStartPosition.CenterParent,
            ClientSize = new Size(480, 480),
            MaximizeBox = false,
            MinimizeBox = false,
            ShowInTaskbar = false
        };

        dialog.Controls.Add(new PictureBox
        {
            Image = image,
            Location = new Point(16, 30),
            Size = new Size(165, 200),
            SizeMode = PictureBoxSizeMode.Zoom
        });
        var titleLabel = new Label
        {
            Text = "nBodySim",
            Location = new Point(198, 34),
            AutoSize = true,
            Font = titleFont
        };
        dialog.Controls.Add(titleLabel);

        var portLabel = new Label
        {
            Text = ".NET 10 Windows Forms port",
            Location = new Point(198, titleLabel.Bottom + 12),
            AutoSize = true,
            MaximumSize = new Size(250, 0)
        };
        dialog.Controls.Add(portLabel);

        dialog.Controls.Add(new Label
        {
            Text = "Original C++ software\nCopyright © 2003, 2004 Lewis Sellers\nIntrafoundation Software",
            Location = new Point(198, portLabel.Bottom + 16),
            AutoSize = true,
            MaximumSize = new Size(250, 0)
        });

        var okButton = new Button
        {
            Text = "OK",
            DialogResult = DialogResult.OK,
            Location = new Point(340, 400),
            Size = new Size(124, 64),
            Padding = new Padding(24, 12, 24, 12),
            TextAlign = ContentAlignment.MiddleCenter
        };
        dialog.Controls.Add(okButton);
        dialog.AcceptButton = okButton;
        dialog.CancelButton = okButton;
        dialog.ShowDialog(this);
    }

    private void PaintWorld(object? sender, PaintEventArgs e)
    {
        var graphics = e.Graphics;
        graphics.SmoothingMode = SmoothingMode.AntiAlias;
        graphics.Clear(SystemColors.Window);
        var width = _canvas.ClientSize.Width;
        var height = _canvas.ClientSize.Height;
        if (width <= 0 || height <= 0)
        {
            return;
        }

        var spanX = Math.Max(1, _simulation.Right - _simulation.Left);
        var spanY = Math.Max(1, _simulation.Bottom - _simulation.Top);
        var scale = Math.Min(width, height) / Math.Max(spanX, spanY);

        if (_showGrid)
        {
            using var gridPen = new Pen(Color.FromArgb(242, 246, 242));
            for (var position = -10000; position <= 10000; position += 100)
            {
                DrawWorldLine(graphics, gridPen, -10000, position, 10000, position, scale);
                DrawWorldLine(graphics, gridPen, position, -10000, position, 10000, scale);
            }
        }

        using (var borderPen = new Pen(Color.Black))
        {
            DrawWorldLine(graphics, borderPen, -100, -100, 100, -100, scale);
            DrawWorldLine(graphics, borderPen, 100, -100, 100, 100, scale);
            DrawWorldLine(graphics, borderPen, 100, 100, -100, 100, scale);
            DrawWorldLine(graphics, borderPen, -100, 100, -100, -100, scale);
        }
        if (_simulation.ViewportDistance > 0)
        {
            var d = _simulation.ViewportDistance;
            using var viewportPen = new Pen(Color.FromArgb(230, 230, 230));
            DrawWorldLine(graphics, viewportPen, -d, -d, d, -d, scale);
            DrawWorldLine(graphics, viewportPen, d, -d, d, d, scale);
            DrawWorldLine(graphics, viewportPen, d, d, -d, d, scale);
            DrawWorldLine(graphics, viewportPen, -d, d, -d, -d, scale);
        }

        if (_showNumericData)
        {
            DrawNumericData(graphics);
        }
        if (_simulation.ViewableCount == 0)
        {
            TextRenderer.DrawText(graphics, "No bodies in viewport.",
                Font, _canvas.ClientRectangle, ForeColor, TextFormatFlags.HorizontalCenter |
                TextFormatFlags.VerticalCenter);
        }

        for (var i = 0; i < _simulation.Bodies.Count; i++)
        {
            var body = _simulation.Bodies[i];
            var center = WorldToScreen(body.Position[0], body.Position[1], scale);
            var radius = Math.Max(1, (int)Math.Round(body.Radius * scale));
            var bounds = new Rectangle(center.X - radius, center.Y - radius, radius * 2, radius * 2);

            if (i == _selectedBody)
            {
                using var selectionPen = new Pen(Color.LimeGreen);
                var selectionRadius = Math.Max(radius + 2, (int)Math.Round(radius * 1.2));
                graphics.DrawEllipse(selectionPen, center.X - selectionRadius, center.Y - selectionRadius,
                    selectionRadius * 2, selectionRadius * 2);
                if (_draggingMomentum)
                {
                    graphics.DrawLine(selectionPen, _momentumStart, _momentumEnd);
                }
            }

            var color = GetBodyColor(i, body);
            using var bodyBrush = new SolidBrush(color);
            using var bodyPen = new Pen(color);
            if (body.Collision)
            {
                bodyBrush.Color = Color.Red;
                bodyPen.Color = Color.Red;
            }
            if (body.Ignore)
            {
                graphics.DrawEllipse(bodyPen, bounds);
            }
            else
            {
                graphics.FillEllipse(bodyBrush, bounds);
            }
            if (body.Mass <= 0)
            {
                graphics.DrawEllipse(bodyPen, center.X - radius - 2, center.Y - radius - 2,
                    (radius + 2) * 2, (radius + 2) * 2);
                graphics.DrawEllipse(bodyPen, center.X - radius - 4, center.Y - radius - 4,
                    (radius + 4) * 2, (radius + 4) * 2);
            }
        }
    }

    private Color GetBodyColor(int index, SimBody body)
    {
        if (body.Collision)
        {
            return Color.Red;
        }
        if (_simulation.ThreeDimensions)
        {
            var denominator = body.Position[2] >= 0 ? _simulation.Distant : _simulation.Close;
            if (Math.Abs(denominator) < double.Epsilon)
            {
                return Color.Black;
            }
            var component = Math.Clamp((int)(body.Position[2] / denominator * 255), 0, 255);
            return body.Position[2] >= 0
                ? Color.FromArgb(component, component / 2, component / 2)
                : Color.FromArgb(0, 0, component);
        }
        var shade = _simulation.Bodies.Count <= 1
            ? 0
            : (int)((double)index / _simulation.Bodies.Count * 240);
        return Color.FromArgb(shade, shade, shade);
    }

    private void DrawNumericData(Graphics graphics)
    {
        using var font = new Font(FontFamily.GenericMonospace, 8);
        var lines = new List<string>
        {
            $"center=({_simulation.CenterX:F2}, {_simulation.CenterY:F2}) " +
            $"bounds=[{_simulation.Left:F2}, {_simulation.Top:F2}, {_simulation.Right:F2}, {_simulation.Bottom:F2}]"
        };
        var count = Math.Min(_simulation.Bodies.Count, 24);
        for (var i = 0; i < count; i++)
        {
            var body = _simulation.Bodies[i];
            lines.Add($"{(i == _selectedBody ? '*' : ' ')}{i,3}] " +
                      $"{(body.Collision ? 'C' : ' ')}{(body.Ignore ? 'I' : ' ')} " +
                      $"{body.Mass:E2}m {body.Radius:E2}r " +
                      $"[{body.Position[0]:F1}x {body.Position[1]:F1}y {body.Position[2]:F1}z] " +
                      $"{body.Velocity[0]:F3}xv {body.Velocity[1]:F3}yv {body.Velocity[2]:F3}zv");
        }
        if (_simulation.Bodies.Count > count)
        {
            lines.Add($"... {_simulation.Bodies.Count - count} more bodies");
        }
        var y = 8;
        var lineHeight = TextRenderer.MeasureText(graphics, "Ag", font).Height + 2;
        foreach (var line in lines)
        {
            TextRenderer.DrawText(graphics, line, font, new Point(8, y), Color.Black);
            y += lineHeight;
        }
    }

    private Point WorldToScreen(double x, double y, double scale)
    {
        var screenX = (x - _simulation.CenterX) * scale + _canvas.ClientSize.Width / 2.0;
        var screenY = (y - _simulation.CenterY) * scale + _canvas.ClientSize.Height / 2.0;
        return new Point(ClampToInt(screenX), ClampToInt(screenY));
    }

    private void DrawWorldLine(Graphics graphics, Pen pen, double x1, double y1, double x2, double y2,
        double scale)
    {
        graphics.DrawLine(pen, WorldToScreen(x1, y1, scale), WorldToScreen(x2, y2, scale));
    }

    private static int ClampToInt(double value)
    {
        if (double.IsNaN(value))
        {
            return 0;
        }
        return (int)Math.Clamp(value, int.MinValue / 2.0, int.MaxValue / 2.0);
    }

    private double CurrentScale()
    {
        var spanX = Math.Max(1, _simulation.Right - _simulation.Left);
        var spanY = Math.Max(1, _simulation.Bottom - _simulation.Top);
        return Math.Min(_canvas.ClientSize.Width, _canvas.ClientSize.Height) / Math.Max(spanX, spanY);
    }

    private void CanvasMouseDown(object? sender, MouseEventArgs e)
    {
        if (e.Button == MouseButtons.Left)
        {
            var scale = CurrentScale();
            for (var i = 0; i < _simulation.Bodies.Count; i++)
            {
                var body = _simulation.Bodies[i];
                var center = WorldToScreen(body.Position[0], body.Position[1], scale);
                var radius = Math.Max(1, body.Radius * scale);
                if (Math.Sqrt(Math.Pow(center.X - e.X, 2) + Math.Pow(center.Y - e.Y, 2)) <= radius)
                {
                    _selectedBody = i;
                }
            }
        }
        else if (e.Button == MouseButtons.Right)
        {
            _momentumStart = e.Location;
            _momentumEnd = e.Location;
            _draggingMomentum = true;
        }
        _canvas.Invalidate();
    }

    private void CanvasMouseMove(object? sender, MouseEventArgs e)
    {
        if (_draggingMomentum)
        {
            _momentumEnd = e.Location;
            _canvas.Invalidate();
        }
    }

    private void CanvasMouseUp(object? sender, MouseEventArgs e)
    {
        if (e.Button != MouseButtons.Right || !_draggingMomentum)
        {
            return;
        }

        _draggingMomentum = false;
        var dx = e.X - _momentumStart.X;
        var dy = e.Y - _momentumStart.Y;
        var magnitude = 0.005 * (640 / Math.Max(1, _simulation.Right - _simulation.Left) +
                                 480 / Math.Max(1, _simulation.Bottom - _simulation.Top));
        Selected.Velocity[0] = magnitude * dx;
        Selected.Velocity[1] = magnitude * dy;
        _canvas.Invalidate();
    }

    private void CanvasMouseWheel(object? sender, MouseEventArgs e)
    {
        _simulation.ViewportDistance = Math.Clamp(
            _simulation.ViewportDistance + e.Delta / SystemInformation.MouseWheelScrollDelta * 100,
            0, 10000);
        _simulation.IgnoreAll(false);
        _simulation.RecomputeViewport();
    }

    private sealed class RenderPanel : Panel
    {
        public RenderPanel()
        {
            DoubleBuffered = true;
            ResizeRedraw = true;
            TabStop = true;
        }
    }
}
