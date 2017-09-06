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
function tripal_drush_command() {
  $items = array();
  $items['install-prepare'] = array(
    'description' => "Installs Chado and Prepares the Site.",
    'arguments' => array(
      'chado-version' => 'Which version of chado should be installed: 1.3, 1.2 or 1.11',
    ),
    'options' => array(
      'version' => array(
        'description' => 'The Chado version.',
        'example-value' => '1.3',
      ),
    ),
    'examples' => array(
      'drush tripal-install --version=1.3' => 'Install Chado Version 1.3 and prepare your tripal site.',
    ),
    'aliases' => array('tripal-install'),
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

function drush_tripal_install_prepare() {
  exec('wget https://www.drupal.org/files/projects/drupal-7.56.tar.gz');
  exec('tar -zxvf drupal-7.56.tar.gz');
  exec('mv drupal-7.56/* ./');
  exec('mv drupal-7.56/.htaccess ./');

  //settings.php input
  $sitename  = drush_prompt(dt('Name of the site: '));
  $siteemail  = drush_prompt(dt('Admin email for the site: '));
  $username  = drush_prompt(dt('Name for your admin user on the site: '));
  $userpassword  = drush_prompt(dt('Password for the admin user, needs to be complex including numbers and characters, example P@55w0rd: '));

  drush_print(dt(
    "\n\n\n
   Now we need to build the settings.php files \$databases array section. 
   \n In the end it will look like this:
   \n \$databases['default']['default'] = array(
   \n  'driver' => 'pgsql',
   \n  'database' => 'drupal',
   \n  'username' => 'admin',
   \n  'password' => 'password123',
   \n  'host' => '127.0.0.1',
   \n   'prefix' => '',
   \n   'port'=> '5433',
   \n);"
  ));
  $driver = 'pgsql';
  $database = drush_prompt(dt('\'database\' => '));
  $postgresusername = drush_prompt(dt('\'username\' =>'));
  $postgrespassword = drush_prompt(dt('\'password\' => '));
  $host = drush_prompt(dt('\'host\' => '));
  $port = drush_prompt(dt('\'port\'=> '));

  $args = array(
    'standard',
    'install_configure_form.update_status_module=\'array(FALSE,FALSE)\''
  );
  $options = array(
    '--db-url='. $driver . '://' . $postgresusername . ':' .$postgrespassword .
    '@' .$host. ':' . $port . '/' . $database,
    '--account-name='. $username,
    '--account-pass='. $userpassword,
    '--site-mail='. $siteemail,
    '--site-name='. $sitename
  );

  drush_set_option('username', $username);

  drush_invoke_process('@self', 'si', $args, $options);
  print_r("\n\n\n Downloading modules.\n");
  //Download modules
  $args = array('field_group', 'field_group_table', 'field_formatter_class', 'field_formatter_settings',
    'ctools', 'date', 'devel', 'ds', 'link', 'entity', 'libraries', 'redirect',
    'token', 'tripal-7.x-3.0-rc1', 'uuid', 'jquery_update', 'views', 'webform');
  $options = array();
  drush_invoke_process('@self', 'dl', $args, $options);
  print_r("\n\n\n Enabling modules.\n");
  $args2 = array('ctools', 'date', 'devel', 'ds', 'link', 'entity', 'libraries', 'redirect',
    'token', 'uuid', 'jquery_update', 'views', 'webform', 'field_group',
    'field_group_table', 'field_formatter_class', 'field_formatter_settings',
    'views_ui');
  $options2 = array();
  drush_invoke_process('@self', 'en', $args2, $options2);

  print_r("\n\n\n Applying patches.\n");
  //apply patches
  $currentworkingdirectory = getcwd();
  exec('wget --no-check-certificate https://drupal.org/files/drupal.pgsql-bytea.27.patch');
  exec('patch -p1 < drupal.pgsql-bytea.27.patch');
  chdir($currentworkingdirectory . "/sites/all/modules/views");
  exec('patch -p1 < ../tripal/tripal_chado_views/views-sql-compliant-three-tier-naming-1971160-30.patch');
  chdir('.');
  print_r("\n\n\n Enabling the Tripal related modules.");
  $args3 = array('tripal', 'tripal_chado', 'tripal_ds', 'tripal_ws');
  $options3 = array();
  drush_invoke_process('@self', 'en', $args3, $options3);

  print_r("\n\n\n Clear cache.\n");
  drush_invoke_process("@site", "cc", array("all"), array("verbose"));

  print_r("\n\n\n Now installing Chado.\n");
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
  print_r("\n\n\n Adding permissions for the administrator to view, edit, create, and delete all the newly created content types.\n");
  //Get all the content types and add the permissions.
  $permissions = array();
  $bundles = array();
  $conn = pg_pconnect("host=localhost dbname=drupal user=drupal password=drupal");
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
