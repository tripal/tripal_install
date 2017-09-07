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
  $items = array();
  $items['install-prepare'] = array(
    'description' => "Installs Chado and Prepares the Site.",
    'examples' => array(
      'drush tripal-generic-install',
    ),
    'aliases' => array('tripal-generic-install'),
    // No bootstrap at all.
    'bootstrap' => DRUSH_BOOTSTRAP_NONE,
  );
  return $items;
}

/**
 * Implements drush_hook_COMMAND().
 *
 *
 * @see drush_invoke()
 * @see drush.api.php
 */

function drush_tripal_install_install_prepare() {
  //install.php input
  $site_settings = FALSE;
  while(!$site_settings){
    $site_name  = drush_prompt(dt('Name of the site: '));
    //Validate email
    $valid_email = FALSE;
    while(!$valid_email){
      $site_email  = drush_prompt(dt('Admin email for the site: '));
      if (filter_var($site_email, FILTER_VALIDATE_EMAIL)) {
        $valid_email = TRUE;
      }
      else {
        drush_print(dt(
          "\nThis is not a valid email address please re-enter the email address.\n"
        ));
      }
    }
    $username  = drush_prompt(dt('Name for your admin user on the site: '));
    $user_password  = drush_prompt(dt('Password for the admin user, needs to be complex including numbers and characters, example P@55w0rd: '));
    drush_print(dt(
      "\n\n\n
       These are the site settings provided, please review and confirm they are correct: 
       \n Site Name: $site_name 
       \n Site email address: $site_email
       \n Administrator username: $username
       \n Administrator password: $user_password
       \n);"
    ));
    $site_settings  = drush_confirm(dt('Is this information correct?'));
  }
  //settings.php input
  $settings_php = FALSE;
  while(!$settings_php){
    drush_print(dt(
      "\n\n\nNow we need to build the settings.php files \$databases array section. 
     \n In the end it will look like this:
     \n \$databases['default']['default'] = array(
     \n  'driver' => 'pgsql',
     \n  'database' => 'drupal',
     \n  'username' => 'admin',
     \n  'password' => 'password123',
     \n  'host' => 'localhost',
     \n   'prefix' => '',
     \n   'port'=> '5432',
     \n);"
    ));
    $driver = 'pgsql';
    $database = drush_prompt(dt('\'database\' => '));
    $postgres_username = drush_prompt(dt('\'username\' =>'));
    $postgres_password = drush_prompt(dt('\'password\' => '));
    $host = drush_prompt(dt('\'host\' => '));
    $port = drush_prompt(dt('\'port\'=> '));

    drush_print(dt(
      "\n\n\nThis is the database information provided, please review and confirm it is correct: 
       \n Database name: $database 
       \n Database username: $postgres_username
       \n Database user password: $postgres_password
       \n Database host: $host
       \n Database port: $port
       \n);"
    ));
    $settings_php  = drush_confirm(dt('Is this information correct?'));
  }

  //Download and install drupal core then move it up one directory level.
  exec('wget https://www.drupal.org/files/projects/drupal-7.56.tar.gz');
  exec('tar -zxvf drupal-7.56.tar.gz');
  exec('mv drupal-7.56/* ./');
  exec('mv drupal-7.56/.htaccess ./');

  //Arguments for the Drush site-install command.
  $args = array(
    'standard',
    'install_configure_form.update_status_module=\'array(FALSE,FALSE)\''
  );

  //Options for the Drush site-install command.
  $options = array(
    '--db-url='. $driver . '://' . $postgres_username . ':' .$postgres_password .
    '@' .$host. ':' . $port . '/' . $database,
    '--account-name='. $username,
    '--account-pass='. $user_password,
    '--site-mail='. $site_email,
    '--site-name='. $site_name
  );
  drush_invoke_process('@self', 'si', $args, $options);

  print_r("\n\n\nDownloading modules.\n");
  $args = array('field_group', 'field_group_table', 'field_formatter_class', 'field_formatter_settings',
    'ctools', 'date', 'devel', 'ds', 'link', 'entity', 'libraries', 'redirect',
    'token', 'tripal-7.x-3.0-rc1', 'uuid', 'jquery_update', 'views', 'webform');
  $options = array();
  drush_invoke_process('@self', 'dl', $args, $options);

  print_r("\n\n\nEnabling modules.\n");
  $args2 = array('ctools', 'date', 'devel', 'ds', 'link', 'entity', 'libraries', 'redirect',
    'token', 'uuid', 'jquery_update', 'views', 'webform', 'field_group',
    'field_group_table', 'field_formatter_class', 'field_formatter_settings',
    'views_ui');
  $options2 = array();
  drush_invoke_process('@self', 'en', $args2, $options2);

  print_r("\n\n\nApplying patches.\n");
  //apply patches
  $current_working_directory = getcwd();
  exec('wget --no-check-certificate https://drupal.org/files/drupal.pgsql-bytea.27.patch');
  exec('patch -p1 < drupal.pgsql-bytea.27.patch');
  chdir($current_working_directory . "/sites/all/modules/views");
  exec('patch -p1 < ../tripal/tripal_chado_views/views-sql-compliant-three-tier-naming-1971160-30.patch');
  chdir('.');

  print_r("\n\n\nEnabling Tripal modules.\n");
  $args3 = array('tripal', 'tripal_chado', 'tripal_ds', 'tripal_ws');
  $options3 = array();
  drush_invoke_process('@self', 'en', $args3, $options3);

  print_r("\n\n\nClear cache.\n");
  drush_invoke_process("@site", "cc", array("all"), array("verbose"));

  print_r("\n\n\nInstalling Chado.\n");
  $options = array(
    'Install Chado v1.3' => 'Install Chado v1.3',
    'Install Chado v1.2' => 'Install Chado v1.2',
    'Install Chado v1.11' => 'Install Chado v1.11',
  );
  $version = drush_choice($options, dt('Which version of Chado would you like installed?'));

  if ($version) {
    if($version == 'Install Chado v1.3') {
      drush_invoke_process('@self', 'php-eval', array("module_load_include('inc', 'tripal_chado', 'includes/tripal_chado.install'); tripal_chado_load_drush_submit('Install Chado v1.3');"), array());
    }
    elseif($version == 'Install Chado v1.2') {
      drush_invoke_process('@self', 'php-eval', array("module_load_include('inc', 'tripal_chado', 'includes/tripal_chado.install'); tripal_chado_load_drush_submit('Install Chado v1.2');"), array());
    }
    elseif($version == 'Install Chado v1.11') {
      drush_invoke_process('@self', 'php-eval', array("module_load_include('inc', 'tripal_chado', 'includes/tripal_chado.install'); tripal_chado_load_drush_submit('Install Chado v1.11');"), array());
    }
  }
  drush_invoke_process('@self', 'php-eval', array("module_load_include('inc', 'tripal', 'tripal.drush'); drush_tripal_trp_run_jobs_install(" . $username . ");"), array());

  print_r("\n\n\n Now preparing the site by creating content types.\n");
  drush_invoke_process('@self', 'php-eval', array("module_load_include('inc', 'tripal_chado', 'includes/setup/tripal_chado.setup'); tripal_chado_prepare_drush_submit();"), array());
  drush_invoke_process('@self', 'php-eval', array("module_load_include('inc', 'tripal', 'tripal.drush'); drush_tripal_trp_run_jobs_install(" . $username . ");"), array());

  //Get all the content types and add the permissions.
  print_r("\n\n\nAdding permissions for the administrator to view, edit, create, and delete all the newly created content types.\n");
  $permissions = array();
  $bundles = array();
  $conn = pg_pconnect("host=$host dbname=$database user=$postgres_username password=$postgres_password");
  if (!$conn) {
    echo "An error occurred.\n";
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
  foreach($bundles as $bundles=>$bundle){
    array_push($permissions, ' view ' . $bundle[0], ' create ' . $bundle[0],
      ' edit ' . $bundle[0],  ' delete ' . $bundle[0]);
  }
  $string_permissions = implode(",", $permissions);
  $args4 = array('administrator', $string_permissions);
  $options4 = array();
  drush_invoke_process('@self', 'role-add-perm', $args4, $options4);

}
