INSERT INTO roles (name, description)
VALUES
    ('terrain', 'Utilisateur travaillant sur le terrain'),
    ('coordinateur', 'Coordinateur des équipes et opérations'),
    ('admin', 'Administrateur de la plateforme')
ON CONFLICT (name) DO NOTHING;