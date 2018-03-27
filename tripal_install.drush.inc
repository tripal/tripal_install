<?php
/**
 * @file
 * Example drush command.
 *
 * To run this *fun* command, execute `sudo drush --include=./examples mmas`
 * from within your drush directory.
 *
 * See `drush topic docs-commands` for more information about command authoring.
 *
 * You can copy this file to any of the following
 *   1. A .drush folder in your HOME folder.
 *   2. Anywhere in a folder tree below an active module on your site.
 *   3. /usr/share/drush/commands (configurable)
 *   4. In an arbitrary folder specified with the --include option.
 *   5. Drupal's /drush or /sites/all/drush folders, or in the /drush
 *        folder in the directory above the Drupal root.
 */
/**
 * Implements hook_drush_command().
 *
 * In this hook, you specify which commands your
 * drush module makes available, what it does and
 * description.
 *
 * Notice how this structure closely resembles how
 * you define menu hooks.
 *
 * See `drush topic docs-commands` for a list of recognized keys.
 */
function tripal_install_drush_command() {
  $items = [];
  $items['install-prepare'] = [
    'description' => "Installs Chado and Prepares the Site.",
    'examples' => [
      'drush tripal-generic-install',
    ],
    'aliases' => ['tripal-generic-install'],
    // No bootstrap at all.
    'bootstrap' => DRUSH_BOOTSTRAP_NONE,
    'options' => [
      // (int) whether to run the command interactively
      'interactive' => 'Whether to run the command interactively. Choose between 1 or 0 for true and false respectively',

      // (string) Site information
      'site-name' => 'Specifies the site name',
      'site-email' => 'Specifies the site email',
      'admin-username' => 'Specifies the administrator\'s username',
      'admin-password' => 'Specifies the administrator\'s password',

      // (string) Database information
      'db-name' => 'Specifies the database name',
      'db-host' => 'Specifies the database host',
      'db-port' => 'Specifies the database port',
      'db-username' => 'Specifies the database username',
      'db-password' => 'Specifies the database password',

      // (int) Chado version. Options from 0 to 3 corresponding to the following
      // 0 - cancel
      // 1 - chado 1.3
      // 2 - chado 1.2
      // 3 - chado 1.11
      'chado-version' => 'Specifies the chado database version. Available options are 0 for skip installation, 1 for chado 1.3, 2 for chado 1.2 and 3 for chado 1.11.',
    ],
  ];
  return $items;
}

/**
 * Implements drush_hook_COMMAND().
 *
 * @see drush_invoke()
 * @see drush.api.php
 */
function drush_tripal_install_install_prepare() {
  $interactive = drush_get_option('interactive', TRUE);
  try {
    if ($interactive === TRUE || $interactive === 1) {
      $settings = tripal_install_get_settings_interactively();
    }
    else {
      $settings = tripal_install_get_inline_settings();
    }

    // Do the install
    tripal_install_perform_tasks($settings);
  } catch (Exception $exception) {
    drush_log($exception->getMessage(), 'error');
    return;
  }
}

/**
 * Get settings from the command options.
 *
 * @return array
 * @throws \Exception
 */
function tripal_install_get_inline_settings() {
  // Validate all options
  $options = [
    'site-name' => drush_get_option('site-name'),
    'site-email' => drush_get_option('site-email'),
    'admin-username' => drush_get_option('admin-username'),
    'admin-password' => drush_get_option('admin-password'),
    'db-host' => drush_get_option('db-host'),
    'db-port' => drush_get_option('db-port'),
    'db-name' => drush_get_option('db-name'),
    'db-username' => drush_get_option('db-username'),
    'db-password' => drush_get_option('db-password'),
    'chado-version' => drush_get_option('chado-version'),
  ];

  $has_errors = FALSE;
  foreach ($options as $option => $value) {
    if (!$value) {
      drush_log("Option --{$option} is required in non-interactive mode.", 'error');
      $has_errors = TRUE;
    }
  }

  if ($has_errors) {
    throw new Exception('Installation failed. Please review errors above.');
  }

  return [
    'postgres_username' => $options['db-username'],
    'postgres_password' => $options['db-password'],
    'host' => $options['db-host'],
    'database' => $options['db-name'],
    'port' => $options['db-port'],
    'username' => $options['admin-username'],
    'user_password' => $options['admin-password'],
    'site_email' => $options['site-email'],
    'site_name' => $options['site-name'],
  ];
}

/**
 * Get the settings by interactively prompting the user.
 *
 * @return array
 */
function tripal_install_get_settings_interactively() {
  //install.php input
  $site_settings = FALSE;
  while (!$site_settings) {
    $site_name = drush_prompt(dt('Name of the site '));
    //Validate email
    $valid_email = FALSE;
    while (!$valid_email) {
      $site_email = drush_prompt(dt('Admin email for the site '));
      if (filter_var($site_email, FILTER_VALIDATE_EMAIL)) {
        $valid_email = TRUE;
      }
      else {
        drush_print(dt(""));
        drush_print(dt(
          "This is not a valid email address please re-enter the email address.\n"
        ));
      }
    }
    $username = drush_prompt(dt('Name for your admin user on the site '));
    $user_password = drush_prompt(dt('Password for the admin user, needs to be complex including numbers and characters, example P@55w0rd '));
    drush_print(dt(""));
    drush_print(dt(
      "These are the site settings provided, please review and confirm they are correct 
       Site Name: $site_name 
       Site email address: $site_email
       Administrator username: $username
       Administrator password: $user_password"
    ));
    $site_settings = drush_confirm(dt('Is this information correct?'));
  }
  //settings.php input
  $settings_php = FALSE;
  while (!$settings_php) {
    drush_print(dt(""));
    drush_print(dt(
      "Now we need to setup Drupal to connect to the database you want to use. These settings are added to Drupal's settings.php file.\n"
    ));
    $database = drush_prompt(dt('database name'));
    $postgres_username = drush_prompt(dt('postgres username'));
    $postgres_password = drush_prompt(dt('postgres password'));
    $host = drush_prompt(dt('host, like localhost or 127.0.0.1'));
    $port = drush_prompt(dt('port, usually 5432'));
    drush_print(dt(""));
    drush_print(dt(
      "This is the database information provided, please review and confirm it is correct: 
       Database name: $database 
       Database username: $postgres_username
       Database user password: $postgres_password
       Database host: $host
       Database port: $port"
    ));
    $settings_php = drush_confirm(dt('Is this information correct?'));
  }

  drush_print(dt(""));

  return [
    'postgres_username' => $postgres_username,
    'postgres_password' => $postgres_password,
    'host' => $host,
    'database' => $database,
    'port' => $port,
    'username' => $username,
    'user_password' => $user_password,
    'site_email' => $site_email,
    'site_name' => $site_name,
  ];
}

/**
 * Perform the installation task.
 *
 * @param array $settings
 *
 * @throws \Exception
 */
function tripal_install_perform_tasks(array $settings) {
  $driver = 'pgsql';
  $postgres_username = $settings['postgres_username'];
  $postgres_password = $settings['postgres_password'];
  $host = $settings['host'];
  $database = $settings['database'];
  $port = $settings['port'];
  $username = $settings['username'];
  $user_password = $settings['user_password'];
  $site_email = $settings['site_email'];
  $site_name = $settings['site_name'];

  drush_print(dt("Now installing Drupal.\n"));
  //Download and install drupal core then move it up one directory level.
  tripal_install_exec('wget https://www.drupal.org/files/projects/drupal-7.56.tar.gz');
  tripal_install_exec('tar -zxvf drupal-7.56.tar.gz');
  tripal_install_exec('mv drupal-7.56/* ./');
  tripal_install_exec('mv drupal-7.56/.htaccess ./');

  //Arguments for the Drush site-install command.
  $args = [
    'standard',
    'install_configure_form.update_status_module=\'array(FALSE,FALSE)\'',
  ];

  //Options for the Drush site-install command.
  $options = [
    '--db-url=' . $driver . '://' . $postgres_username . ':' . $postgres_password .
    '@' . $host . ':' . $port . '/' . $database,
    '--account-name=' . $username,
    '--account-pass=' . $user_password,
    '--site-mail=' . $site_email,
    '--site-name=' . $site_name,
  ];
  $install = drush_invoke_process('@self', 'si', $args, $options);
  drush_print(dt(""));
  if ($install === FALSE) {
    throw new Exception("An error occurred when attempting to install Drupal.
                          If you would like to try again please remove any files
                          or tables created during this intial installation attempt.");
  }
  print_r("Downloading modules.\n");
  $args = [
    'field_group',
    'field_group_table',
    'field_formatter_class',
    'field_formatter_settings',
    'ctools',
    'date',
    'devel',
    'ds',
    'link',
    'entity',
    'libraries',
    'redirect',
    'token',
    'tripal-7.x-3.0-rc2',
    'uuid',
    'jquery_update',
    'views',
    'webform',
  ];
  $options = [];
  $download = drush_invoke_process('@self', 'dl', $args, $options);
  if ($download === FALSE) {
    throw new Exception("Unable to download modules.");
  }
  drush_print(dt(""));
  print_r("Enabling modules.\n");
  $args2 = [
    'ctools',
    'date',
    'devel',
    'ds',
    'link',
    'entity',
    'libraries',
    'redirect',
    'token',
    'uuid',
    'jquery_update',
    'views',
    'webform',
    'field_group',
    'field_group_table',
    'field_formatter_class',
    'field_formatter_settings',
    'views_ui',
  ];
  $options2 = [];
  $enable = drush_invoke_process('@self', 'en', $args2, $options2);
  if ($enable === FALSE) {
    throw new Exception('Unable to enable modules');
  }
  drush_print(dt("\n"));
  print_r("Applying patches.\n");
  //apply patches
  $current_working_directory = getcwd();
  tripal_install_exec('wget --no-check-certificate https://drupal.org/files/drupal.pgsql-bytea.27.patch');
  tripal_install_exec('patch -p1 < drupal.pgsql-bytea.27.patch');
  chdir($current_working_directory . "/sites/all/modules/views");
  tripal_install_exec('patch -p1 < ../tripal/tripal_chado_views/views-sql-compliant-three-tier-naming-1971160-30.patch');
  chdir('.');
  drush_print(dt(""));
  print_r("Enabling Tripal modules.\n");
  $args3 = ['tripal', 'tripal_chado', 'tripal_ds', 'tripal_ws'];
  $options3 = [];
  $tripal = drush_invoke_process('@self', 'en', $args3, $options3);
  drush_print(dt(""));
  if ($tripal === FALSE) {
    throw new Exception("An error occurred when attempting to enable Tripal.
                        Please navigate to your new site and finish the 
                        installation process from the 'Install Tripal' section
                        as described in the online help, found here
                        http://tripal.info/tutorials/v3.x/installation/tripal");
  }
  drush_print(dt(""));
  print_r("Clear cache.\n");
  $clear_cache = drush_invoke_process("@site", "cc", ["all"], ["verbose"]);
  if ($clear_cache === FALSE) {
    throw new Exception("Unable to clear the site's cache.");
  }
  drush_print(dt(""));
  print_r("Installing Chado.\n");

  if (drush_get_option('interactive', TRUE)) {
    $options = [
      'Install Chado v1.3' => 'Install Chado v1.3',
      'Install Chado v1.2' => 'Install Chado v1.2',
      'Install Chado v1.11' => 'Install Chado v1.11',
    ];
    $version = drush_choice($options, dt('Which version of Chado would you like installed?'));
  }
  else {
    $version = intval(drush_get_option('chado-version', 0));
  }

  if ($version) {
    if ($version == 'Install Chado v1.3' || $version === 1) {
      $chado = drush_invoke_process('@self', 'php-eval', ["module_load_include('inc', 'tripal_chado', 'includes/tripal_chado.install'); tripal_chado_load_drush_submit('Install Chado v1.3');"], []);
    }
    elseif ($version == 'Install Chado v1.2' || $version === 2) {
      $chado = drush_invoke_process('@self', 'php-eval', ["module_load_include('inc', 'tripal_chado', 'includes/tripal_chado.install'); tripal_chado_load_drush_submit('Install Chado v1.2');"], []);
    }
    elseif ($version == 'Install Chado v1.11' || $version === 3) {
      $chado = drush_invoke_process('@self', 'php-eval', ["module_load_include('inc', 'tripal_chado', 'includes/tripal_chado.install'); tripal_chado_load_drush_submit('Install Chado v1.11');"], []);
    }
    else {
      $chado = TRUE;
    }
    if ($chado === FALSE) {
      throw new Exception("Failed to install chado");
    }
  }
  drush_invoke_process('@self', 'php-eval', ["module_load_include('inc', 'tripal', 'tripal.drush'); drush_tripal_trp_run_jobs_install(" . $username . ");"], []);
  drush_print(dt(""));
  if (!$tripal) {
    echo "An error occurred when attempting to install Chado. Please navigate to your new site and finish the installation process from the 'Install Tripal' section as described in the online help, found here http://tripal.info/tutorials/v3.x/installation/tripal \n";
    exit;
  }
  print_r("Now preparing the site by creating content types.\n");
  $prepare = drush_invoke_process('@self', 'php-eval', ["module_load_include('inc', 'tripal_chado', 'includes/setup/tripal_chado.setup'); tripal_chado_prepare_drush_submit();"], []);
  drush_invoke_process('@self', 'php-eval', ["module_load_include('inc', 'tripal', 'tripal.drush'); drush_tripal_trp_run_jobs_install(" . $username . ");"], []);
  if (!$prepare) {
    echo "An error occurred when attempting to install Chado. Please navigate to your new site and finish the installation process from the 'Install Tripal' section as described in the online help, found here http://tripal.info/tutorials/v3.x/installation/tripal \n";
    exit;
  }
  //Get all the content types and add the permissions.
  drush_print(dt(""));
  print_r("Adding permissions for the administrator to view, edit, create, and delete all the newly created content types.\n");
  $permissions = [];
  $bundles = [];
  $conn = pg_pconnect("host=$host dbname=$database user=$postgres_username password=$postgres_password");
  if (!$conn) {
    echo "An error occurred when attempting to perimssion the administrator. Please navigate to your new site and add permissions to your new content types here admin/people/permissions.\n";
    exit;
  }
  $result = pg_query($conn, "SELECT name FROM tripal_bundle");
  if (!$result) {
    echo "An error occurred.\n";
    exit;
  }

  while ($row = pg_fetch_row($result)) {
    array_push($bundles, $row);
  }
  foreach ($bundles as $bundle) {
    array_push($permissions, ' view ' . $bundle[0], ' create ' . $bundle[0],
      ' edit ' . $bundle[0], ' delete ' . $bundle[0]);
  }
  $string_permissions = implode(",", $permissions);
  $args4 = ['administrator', $string_permissions];
  $options4 = [];
  drush_invoke_process('@self', 'role-add-perm', $args4, $options4);

  drush_print(dt(""));
  drush_print(dt("Installation is now complete. You may navigate to your new site. For more information on using Tripal please see the installation guide on tripal.info."));
}

/**
 * A wrapper around exec to detect commands that did not complete successfully.
 *
 * @param $command
 *
 * @throws \Exception
 */
function tripal_install_exec($command) {
  exec($command, $output, $return_var);

  if ($return_var !== 0) {
    throw new Exception("$command failed to execute successfully.");
  }
}
