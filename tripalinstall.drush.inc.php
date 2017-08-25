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
function tripalinstall_drush_command() {
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
 * The command callback is where the action takes place.
 *
 * The function name should be same as command name but with dashes turned to
 * underscores and 'drush_commandfile_' prepended, where 'commandfile' is
 * taken from the file 'commandfile.drush.inc', which in this case is
 * 'sandwich'. Note also that a simplification step is also done in instances
 * where the commandfile name is the same as the beginning of the command name,
 * "drush_example_example_foo" is simplified to just "drush_example_foo".
 * To also implement a hook that is called before your command, implement
 * "drush_hook_pre_example_foo".  For a list of all available hooks for a
 * given command, run drush in --debug mode.
 *
 * If for some reason you do not want your hook function to be named
 * after your command, you may define a 'callback' item in your command
 * object that specifies the exact name of the function that should be
 * called.
 *
 * In this function, all of Drupal's API is (usually) available, including
 * any functions you have added in your own modules/themes.
 *
 * @see drush_invoke()
 * @see drush.api.php
 */

function drush_tripalinstall_install_prepare() {

  $makefile = drush_prompt(dt('Name of the make file you are using, for example tripal-7.x-3.x.'));
  $sitename  = drush_prompt(dt('Name of the site: '));
  $siteemail  = drush_prompt(dt('Admin email for the site: '));
  $username  = drush_prompt(dt('Name admin user: '));
  $userpassword  = drush_prompt(dt('Password for the admin user, needs to be complex including numbers and characters, example P@55w0rd: '));

  drush_print(dt(
    "\n\n\n
   Now we need to build the settings.php files \$databases array section. 
   \n In the end it will look like this:
   \n \$databases['default']['default'] = array(
   \n  'driver' => 'pgsql',
   \n  'database' => 'drupal2',
   \n  'username' => 'drupal',
   \n  'password' => 'drupal',
   \n  'host' => '127.0.0.1',
   \n   'prefix' => '',
   \n   'port'=> '5433',
   \n);"
  ));
  $driver = drush_prompt(dt('\'driver\' =>'));
  $database = drush_prompt(dt('\'database\' => '));
  $postgresusername = drush_prompt(dt('\'username\' =>'));
  $postgrespassword = drush_prompt(dt('\'password\' => '));
  $host = drush_prompt(dt('\'host\' => '));
  $port = drush_prompt(dt('\'port\'=> '));

  $args = array();
  $options = array(
    '--makefile='.$makefile,
    '--db-url='. $driver . '://' . $postgresusername . ':' .$postgrespassword .
    '@' .$host. ':' . $port . '/' . $database,
    '--account-name='. $username,
    '--account-pass='. $userpassword,
    '--site-mail='. $siteemail,
    '--site-name='. $sitename,
    '--no-server=yes',
    '--yes'
  );

  drush_invoke_process('@self', 'qd', $args, $options);

  $drupaldirectory = drush_prompt(dt("\n\n\n Name of the directory that was created with your Drupal installation.
  This can be quickly found by scrolling up and finding the line that starts with
  'You are about to create a' within the url listed you will see 'quick-drupal-' and a series of numbers.
  Grab the entire string, like this 'quick-drupal-20170824111709' "));

  exec('mv '.$drupaldirectory.'/drupal/.htaccess ./');
  exec('mv '.$drupaldirectory.'/.htaccess ./');
  exec('mv '.$drupaldirectory.'/drupal/* ./');
  exec('mv '.$drupaldirectory.'/drupal/sites ./');
  exec('mv '.$drupaldirectory.'/* ./');

  $args = array('field_group', 'field_group_table', 'field_formatter_class', 'field_formatter_settings');
  $options = array();
  drush_invoke_process('@self', 'dl', $args, $options);

  $args = array('field_group', 'field_group_table', 'field_formatter_class', 'field_formatter_settings',
    'views_ui', 'tripal_chado', 'tripal_ds', 'tripal_ws');
  $options = array();
  drush_invoke_process('@self', 'en', $args, $options);

  drush_set_option('username', $username);

  $options = array(
    'Install Chado v1.3' => 'Install Chado v1.3',
    'Install Chado v1.2' => 'Install Chado v1.2',
    'Install Chado v1.11' => 'Install Chado v1.11',
  );
  $version = drush_choice($options, dt('Which version of Chado would you like installed?'));

  if ($version) {
    module_load_include('inc', 'tripal_chado', 'includes/tripal_chado.install');
    tripal_chado_load_drush_submit($version);
  }
  module_load_include('inc', 'tripal', 'tripal.drush');
  drush_tripal_trp_run_jobs();

  module_load_include('inc', 'tripal_chado', 'includes/setup/tripal_chado.setup');
  tripal_chado_prepare_drush_submit();

  module_load_include('inc', 'tripal', 'tripal.drush');
  drush_tripal_trp_run_jobs();
}
